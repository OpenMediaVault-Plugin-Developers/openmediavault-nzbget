<?php

/**
 * Copyright (C) 2010-2012 Ian Moore <imooreyahoo@gmail.com>
 * Copyright (C) 2013-2015 OpenMediaVault Plugin Developers.
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
 //

namespace OMV\Engined\Rpc;

use DateTime;
use Exception;
use OMV\Config\Database;
use OMV\Engine\Notify\Dispatcher;
use OMV\Rpc\ServiceAbstract;
use OMV\Rpc\ServiceManager as RpcServiceManager;
use OMV\System\Process;
use SplFileObject;

class NZBGet extends ServiceAbstract
{
    private $database;

    private $dataModelPath = 'conf.service.nzbget';

    private $dispatcher;

    private $eventMessagePath = 'org.openmediavault.conf.services.nzbget';

    private $nzbgetSettingsFile = "/usr/share/nzbget/nzbget.conf";

    public function __construct(Database $database = null, Dispatcher $dispatcher = null)
    {
        $this->database = $database ?: Database::getInstance();
        $this->dispatcher = $dispatcher ?: Dispatcher::getInstance();
    }

    public function getName()
    {
        return 'NZBGet';
    }

    public function initialize()
    {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
    }

    public function getSettings($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);

        $object = $this->database->get($this->dataModelPath);

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

        $object->add('port', 'integer');
        $object->set('port', intval($port));

        return $object->getAssoc();
    }

    public function setSettings($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        $this->validateMethodParams($params, 'rpc.nzbget.setsettings');

        $object = $this->database->get($this->dataModelPath);

        if (empty($params['mntentref'])) {
            throw new Exception('You must choose a folder to store downloads.');
        }

        if (!empty($params['main-dir'])) {
                $params['main-dir'] = rtrim($params['main-dir'], DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
        }

        $db = \OMV\Config\Database::getInstance();

        $deviceDir = $db->getAssoc('conf.system.sharedfolder', $params['mntentref']);
        $deviceDir = $db->getAssoc('conf.system.filesystem.mountpoint', $deviceDir['mntentref'])['dir'] . '/' . $deviceDir['reldirpath'];

        if (stripos(strrev($deviceDir), "/") === 0) {
            $dbFolder = sprintf("{$deviceDir}%s", $params['main-dir']);
        } else {
            $dbFolder = sprintf("{$deviceDir}/%s", $params['main-dir']);
        }

        if (file_exists($this->nzbgetSettingsFile)) {
            // Change valuse
            $cmd="chmod 777 {$dbFolder} -R;sed -i 's#^MainDir.*#MainDir=" . $dbFolder . "#g' /usr/share/nzbget/nzbget.conf";

            if (0 !== $this->exec($cmd, $output)) {
                throw new Exception($cmd, implode(PHP_EOL, $output));
            }
        }

        $object->setAssoc($params);

        $this->database->set($object);
        $this->dispatcher->notify(OMV_NOTIFY_MODIFY, $this->eventMessagePath, $object);

        return $object->getAssoc();
    }
}
