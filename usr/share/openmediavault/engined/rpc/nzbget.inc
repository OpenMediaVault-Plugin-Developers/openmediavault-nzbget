<?php

/**
 * Copyright (C) 2013-2015 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

require_once "openmediavault/config.inc";
require_once "openmediavault/error.inc";
require_once "openmediavault/notify.inc";
require_once "openmediavault/object.inc";
require_once "openmediavault/product.inc";
require_once "openmediavault/rpcservice.inc";
require_once "openmediavault/system.inc";
require_once "openmediavault/util.inc";

class OMVRpcServiceNZBGet extends OMVRpcServiceAbstract
{
    private $nzbgetSettingsFile = "/usr/share/nzbget/nzbget.conf";

    private function getPluginName()
    {
        return strtolower($this->getName());
    }

    private function getEventMessagePath()
    {
        return sprintf("org.openmediavault.service.%s", $this->getPluginName());
    }

    private function getXPath()
    {
        return sprintf("/config/services/%s", $this->getPluginName());
    }

    public function getName()
    {
        return "NZBGet";
    }

    public function initialize()
    {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
    }

    public function getSettings($params, $context)
    {
        global $xmlConfig;

        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        $port = 6789;

        // Get current port as set in NZBGet config file
        if (file_exists($this->nzbgetSettingsFile)) {
            $config = parse_ini_file(
                $this->nzbgetSettingsFile,
                true,
                INI_SCANNER_RAW
            );

            if (!empty($config["ControlPort"])) {
                $port = $config["ControlPort"];
            }
        }

        $object["enable"]                   = boolval($object["enable"]);
        $object["showtab"]                  = boolval($object["showtab"]);
        $object["port"]                     = intval($port);
        $object["download.sharedfolderref"] = $object["download.sharedfolderref"];
        $object["main-dir"]                 = $object["main-dir"];

        return $object;
    }

    public function setSettings($params, $context)
    {
        global $xmlConfig;

        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "enable"                      : { "type" : "boolean" },
                    "showtab"                     : { "type" : "boolean" },
                    "port"                        : { "type" : "integer" },
                    "download.sharedfolderref"    : {' . $GLOBALS["OMV_JSONSCHEMA_UUID_NONE"] . '},
                    "folder"                      : { "type" : "string", "optional" : true },
                    "main-dir"                    : { "type" : "string" }
                }
            }'
        );

        // Get the absolute shared folder path.
        $mainpath = OMVRpc::exec("ShareMgmt", "getPath", array(
            "uuid" => $params["download.sharedfolderref"]), $context);

        if (stripos(strrev($mainpath), "/") === 0) {
            $dbFolder = sprintf("{$mainpath}/%s", $params['main-dir']);
        } else {
            $dbFolder = sprintf("{$mainpath}%s", $params['main-dir']);
        }

        if (file_exists($this->nzbgetSettingsFile)) {
            // Change valuse
            if($params["main-dir"] !== '') {
                $cmd="sed -i 's#^MainDir.*#MainDir=" . $mainpath . $params["main-dir"] . "#g' /usr/share/nzbget/nzbget.conf";
            }else{
                $cmd="sed -i 's#^MainDir.*#MainDir=" . $mainpath . "#g' /usr/share/nzbget/nzbget.conf";
            }

            if (0 !== $this->exec($cmd, $output)) {
                throw new OMVException(
                    OMVErrorMsg::E_EXEC_FAILED,
                    $cmd,
                    implode(PHP_EOL, $output)
                );
            }
        }

        $object = [
            "enable"                     => array_boolval($params, "enable"),
            "showtab"                    => array_boolval($params, "showtab"),
            "port"                       => $params["port"],
            "download.sharedfolderref"   => $params["download.sharedfolderref"],
            "folder"                     => $dbFolder,
            "main-dir"                   => $params["main-dir"]
        ];

        if (false === $xmlConfig->replace($this->getXPath(), $object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            $object
        );

        return $object;
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceNZBGet());
