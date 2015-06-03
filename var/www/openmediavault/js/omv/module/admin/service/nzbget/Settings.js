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

// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/form/plugin/LinkedFields.js")
// require("js/omv/form/field/SharedFolderComboBox.js")

Ext.define("OMV.module.admin.service.nzbget.Settings", {
    extend : "OMV.workspace.form.Panel",
    uses : [
        "OMV.form.field.SharedFolderComboBox"
    ],

    rpcService   : "NZBGet",
    rpcGetMethod : "getSettings",
    rpcSetMethod : "setSettings",

    initComponent: function() {
        this.on("load", function() {
            var checked = this.findField("enable").checked;
            var showtab = this.findField("showtab").checked;
            var parent = this.up("tabpanel");

            if (!parent) {
                return;
            }

            var managementPanel = parent.down("panel[title=" + _("Web Interface") + "]");

            if (managementPanel) {
                checked ? managementPanel.enable() : managementPanel.disable();
                showtab ? managementPanel.tab.show() : managementPanel.tab.hide();
            }
        }, this);

        this.callParent(arguments);
    },

    plugins      : [{
        ptype        : "linkedfields",
        correlations : [{
            name       : [
                "port",
            ],
            properties : "!show"
        },{
            name       : [
                "showbutton",
            ],
            conditions : [
                { name  : "enable", value : false }
            ],
            properties : "!show"
        }]
    }],

    getButtonItems: function() {
        var items = this.callParent(arguments);

        items.push({
            id: this.getId() + "-show",
            xtype: "button",
            name: "showbutton",
            text: _("Open Web Client"),
            icon: "images/nzbget.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            scope: this,
            handler: function() {
                var port = this.getForm().findField("port").getValue();
                var link = "http://" + location.hostname + ":" + port + "/";
                window.open(link, "_blank");
            }
        });

        return items;
    },

    getFormItems : function() {
        var me = this;

        return [{
            xtype    : "fieldset",
            title    : "General settings",
            defaults : {
                labelSeparator : ""
            },
            items : [{
                xtype      : "checkbox",
                name       : "enable",
                fieldLabel : _("Enable"),
                checked    : false
            },{
                xtype      : "checkbox",
                name       : "showtab",
                fieldLabel : _("Show Tab"),
                boxLabel   : _("Show tab containing NZBGet web interface frame."),
                checked    : false
            },{
                xtype: "numberfield",
                name: "port",
                fieldLabel: _("Port"),
                vtype: "port",
                minValue: 1,
                maxValue: 65535,
                allowDecimals: false,
                allowBlank: false,
                value: 6789
            },{
                ptype : "fieldinfo",
                xtype      : "sharedfoldercombo",
                name       : "download.sharedfolderref",
                fieldLabel : _("Shared folder"),
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Make sure the group 'users' has read/write access to the shared folder.")
                }]
            },{
                xtype      : "textfield",
                name       : "main-dir",
                fieldLabel : _("Directory"),
                allowBlank : true,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Directory to store downloads.")
                }]
            },{
                border: false,
                html: "<br />"
            }]
        }];
    },
});

OMV.WorkspaceManager.registerPanel({
    id        : "settings",
    path      : "/service/nzbget",
    text      : _("Settings"),
    position  : 10,
    className : "OMV.module.admin.service.nzbget.Settings"
});
