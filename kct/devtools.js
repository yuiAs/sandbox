(function (_window) {
    _window.kancolle = _window.kancolle || {};
    _window.kancolle.mst = {};
    _window.kancolle.usr = {};
    // 面倒なのでいったん消しておく
    chrome.storage.local.clear();
})(window || this);


(function (_window) {
    var kBaseURI = /^http:\/\/www\.dmm\.com\/netgame\/social\/-\/gadgets\/=\/app_id=854854\/$/;
    chrome.devtools.inspectedWindow.eval("document.baseURI", function (result, exceptionInfo) {
        if (kBaseURI.test(result)) {
            _initExtension(_window.kancolle);
        } else {
            chrome.devtools.network.onNavigated.addListener(function _checker(url) {
                if (kBaseURI.test(url)) {
                    chrome.devtools.network.onNavigated.removeListener(_checker);
                    _initExtension(_window.kancolle);
                }
            });
        }
    });

    function _initExtension(_this) {
        _this.tabId = chrome.devtools.inspectedWindow.tabId;

        _this.updateContents = function () {
            //console.log(Date.now() + ": updateContents");
            var ud = _this.usr.deck;
            var us = _this.usr.ship;

            var r = {};
            for (var a in ud) {
                if (ud.hasOwnProperty(a)) {
                    // とりあえず shallowにしてるけど必要に応じて deepに変更
                    ud[a].ship2 = [];
                    ud[a].ship.forEach(function (element, index, array) {
                        ud[a].ship2.push(us[element]);
                    });
                    r[a] = ud[a];
                }
            }
            chrome.storage.local.set({ deck: r });
        };

        _this.mst.load = function (db) {
            var tx = db.transaction([ "Ship", "Equipment" ], "readonly");
            var tmp1 = {};
            tx.objectStore("Ship").openCursor().onsuccess = function (event) {
                var cur = event.target.result;
                if (cur) {
                    tmp1[cur.value.api_id] = cur.value;
                    cur.continue();
                } else {
                    _this.mst.ship = tmp1;
                }
            };
            var tmp2 = {};
            tx.objectStore("Equipment").openCursor().onsuccess = function (event) {
                var cur = event.target.result;
                if (cur) {
                    tmp2[cur.value.api_id] = cur.value;
                    cur.continue();
                } else {
                    _this.mst.equipment = tmp2;
                }
            };
            _this.mst.db = db;
        };
        _this.mst.update = function (db, tmp1, tmp2) {
            var tx = db.transaction([ "Ship", "Equipment" ], "readwrite");
            var o1 = tx.objectStore("Ship");
            for (var a in tmp1) {
                if (tmp1.hasOwnProperty(a)) {
                    o1.put(tmp1[a]);
                }
            }
            var o2 = tx.objectStore("Equipment");
            for (var a in tmp2) {
                if (tmp2.hasOwnProperty(a)) {
                    o2.put(tmp2[a]);
                }
            }
        };

        // Create a connection to the database
        (function () {
            var req1 = _window.indexedDB.open("MasterData", 1);
            req1.onupgradeneeded = function (event) {
                // Create object stores
                var db = event.target.result;
                db.createObjectStore("Ship", { keyPath: "api_id" });
                db.createObjectStore("Equipment", { keyPath: "api_id" });
            };
            req1.onsuccess = function (event) {
                _this.mst.load(event.target.result);
            };
        })();

        // Create a connection to the background page
        _this.bgPort = chrome.runtime.connect({ name: "registerFilters" });
        _this.bgPort.onMessage.addListener(function (message, sender, sendResponse) {
            var deck = _this.usr.deck;
            if (deck && message.data) {
                var sd = message.data;
                if (!deck[sd.api_id]) {
                    return;
                }

                // api_ship_idx==-1の場合は旗艦以外を解除
                if (sd.api_ship_idx < 0) {
                    deck[sd.api_id].ship.splice(1, (deck[sd.api_id].ship.length - 1));
                } else {
                    // api_ship_id==-1の場合は api_ship_idxの単体解除
                    if (sd.api_ship_id == -1) {
                        deck[sd.api_id].ship.splice(sd.api_ship_idx, 1);
                    } else {
                        var i = deck[sd.api_id].ship.indexOf(sd.api_ship_id|0);
                        if (i == -1) {
                            // 追加，差し替え
                            deck[sd.api_id].ship.splice(sd.api_ship_idx, 1, sd.api_ship_id|0);
                        } else {
                            // 入れ替え
                            deck[sd.api_id].ship[sd.api_ship_idx] = [sd.api_ship_id|0, deck[sd.api_id].ship[i] = deck[sd.api_id].ship[sd.api_ship_idx]][0];
                        }
                    }
                }
                _this.updateContents();
            }
        });
    }
})(window || this);


(function (_window) {
    var _mst = _window.kancolle.mst;
    var _usr = _window.kancolle.usr;

    _usr.updateDeck = function (deck, added) {
        var r = {};
        deck.forEach(function (element, index, array) {
            var a = {};
            a.name = element.api_name;
            a.ship = element.api_ship.filter(function (_element, _index, _array) {
                return (_element > 0);
            });
            if (element.api_mission[2] > 0) {
                a.mission = {};
                a.mission.arrival = element.api_mission[2];
            }
            r[element.api_id] = a;
        });

        if (added) {
            for (var a in r) {
                if (r.hasOwnProperty(a)) {
                    _usr.deck[a] = r[a];
                }
            }
        } else {
            _usr.deck = r || _usr.deck;
        }
    };
    _usr.updateShip = function (ship, added) {
        var mst = _mst.ship;
        var ueq = _usr.equipment;

        var r = {};
        ship.forEach(function (element, index, array) {
            if (mst && mst[element.api_ship_id]) {
                element.api_ship_name = mst[element.api_ship_id].api_name;
            } else {
                element.api_ship_name = "???";
            }
            element.api_slot_name = [];
            element.api_slot_type = [];
            element.api_slot.forEach(function (_element) {
                if (_element > 0) {
                    if (ueq && ueq[_element]) {
                        element.api_slot_name.push(ueq[_element].api_slotitem_name);
                        element.api_slot_type.push(ueq[_element].api_slotitem_type[3]);
                        //ueq[_element].api_owner = element.api_ship_id;
                    }
                }
            });
            // 補強増設スロット
            element.api_slot2_name = [];
            element.api_slot2_type = [];
            if (element.api_slot_ex > 0) {
                element.api_slot2_name.push(ueq[element.api_slot_ex].api_slotitem_name);
                element.api_slot2_type.push(ueq[element.api_slot_ex].api_slotitem_type[3]);
                //ueq[element.api_slot_ex].api_owner = element.api_ship_id;
            }
            r[element.api_id] = element;
        });
 
        if (added) {
            for (var a in r) {
                if (r.hasOwnProperty(a)) {
                    _usr.ship[a] = r[a];
                }
            }
        } else {
            _usr.ship = r || _usr.ship;
        }
    };

    // api_start2
    var kAPIRootStart2 = /^http:\/\/[^\/]+\/kcsapi\/api_start2$/;
    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!kAPIRootStart2.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var sd = JSON.parse(content.substring(7)) || {};
            if (!sd.api_data) {
                return;
            }

            // ship, slotitemは配列のままだと扱いにくいので
            // api_idでアクセスできるような形で持っておくための小細工

            (function () {
                var j1 = sd.api_data.api_mst_ship || [];
                var j2 = sd.api_data.api_mst_shipgraph || [];
                var t = {};
                j1.forEach(function (element, index, array) {
                    t[element.api_id] = element;
                });
                // 内部的には使ってないけどちょっとした確認用に mst_shipgraphを合成
                j2.forEach(function (element, index, array) {
                    for (var a in element) {
                        // mst_shipにはないけど mst_shipgraphには存在するケースがあるので弾く
                        if (element.hasOwnProperty(a) && t[element.api_id] && t[element.api_id].hasOwnProperty(a)) {
                            t[element.api_id][a] = element[a];
                        }
                    }
                });
                _mst.ship = t;
            })();

            (function () {
                var j1 = sd.api_data.api_mst_slotitem || [];
                var t = {};
                j1.forEach(function (element, index, array) {
                    t[element.api_id] = element;
                });
                _mst.equipment = t;
            })();

            // Overwrite
            _window.kancolle.mst.update(_mst.db, _mst.ship, _mst.equipment);
        });
    });

    // slot_item
    var kAPIMemberSlotItem = /^http:\/\/[^\/]+\/kcsapi\/api_get_member\/slot_item$/;
    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!kAPIMemberSlotItem.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var sd = JSON.parse(content.substring(7)) || {};
            if (!sd.api_data) {
                return;
            }

            (function () {
                // slot_itemの api_dataはそれ自体が配列
                var j1 = sd.api_data || [];
                var m1 = _window.kancolle.mst.equipment;
                var t = {};
                j1.forEach(function (element, index, array) {
                    var d = m1 && m1[element.api_slotitem_id];
                    if (d) {
                        element.api_slotitem_name = d.api_name;
                        // 実際には api_type[3]がアイコンの種類 (vo.MasterSlotItemData参照)
                        element.api_slotitem_type = d.api_type;
                    }
                    t[element.api_id] = element;
                });
                _usr.equipment =t;
            })();
         });
    });

    // port
    var kAPIPortPort = /^http:\/\/[^\/]+\/kcsapi\/api_port\/port$/;
    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!kAPIPortPort.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var sd = JSON.parse(content.substring(7)) || {};
            if (!sd.api_data) {
                return;
            }
            _usr.updateShip(sd.api_data.api_ship || []);
            _usr.updateDeck(sd.api_data.api_deck_port || []);
            _window.kancolle.updateContents();
        });
    });

    var kAPIMemberShip2 = /^http:\/\/[^\/]+\/kcsapi\/api_get_member\/ship2$/;
    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!kAPIMemberShip2.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var sd = JSON.parse(content.substring(7)) || {};
            _usr.updateShip(sd.api_data || []);
            _window.kancolle.updateContents();
        });
    });

    // 改造, 装備変更
    var kAPIMemberShip3 = /^http:\/\/[^\/]+\/kcsapi\/api_get_member\/ship3$/;
    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!kAPIMemberShip3.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var sd = JSON.parse(content.substring(7)) || {};
            _usr.updateShip(sd.api_data.api_ship_data || [], true);
            _usr.updateDeck(sd.api_data.api_deck_data || []);
            _window.kancolle.updateContents();
        });
    });

    var kAPIMemberDeck = /^http:\/\/[^\/]+\/kcsapi\/api_get_member\/deck$/;
    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!kAPIMemberDeck.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var sd = JSON.parse(content.substring(7)) || {};
            _usr.updateDeck(sd.api_data || []);
            _window.kancolle.updateContents();
        });
    });

    var kAPIMemberShipDeck = /^http:\/\/[^\/]+\/kcsapi\/api_get_member\/ship_deck$/;
    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!kAPIMemberShipDeck.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var sd = JSON.parse(content.substring(7)) || {};
            if (!sd || !sd.api_data) {
                return;
            }
            _usr.updateShip(sd.api_data.api_ship_data || [], true);
            _usr.updateDeck(sd.api_data.api_deck_data || [], true);
            _window.kancolle.updateContents();
        });
    });

    // 編成展開
    var kAPIHenseiPresetSelect = /^http:\/\/[^\/]+\/kcsapi\/api_req_hensei\/preset_select$/;
    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!kAPIHenseiPresetSelect.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var sd = JSON.parse(content.substring(7)) || {};
            if (!sd || !sd.api_data) {
                return;
            }

            // deckが単発で来るので都合上配列にして updateDeckに渡す
            var deck = [];
            if (sd.api_data) {
                deck.push(sd.api_data);
            }
            _usr.updateDeck(deck, true);
            _window.kancolle.updateContents();
        });
    });

})(window || this);
