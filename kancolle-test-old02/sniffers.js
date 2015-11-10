window.kancolle = window.kancolle || {};

(function (_kancolle) {
    //console.log("sniffers.js", _kancolle);

    _kancolle.master = _kancolle.master || {};
    _kancolle.user = {};

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

            var mst1 = {};
            (function () {
                var j1 = sd.api_data.api_mst_ship || [];
                var j2 = sd.api_data.api_mst_shipgraph || [];

                // ちょっと面倒だけど api_mst_shipと api_mst_shipgraphを合成したデータベースを作る
                // 個別に持っててもいいけどそれはそれで面倒
                var mst = {};
                j1.forEach(function (element, index, array) {
                    mst[element.api_id] = element;
                });
                j2.forEach(function (element, index, array) {
                    // api_mst_shipにない api_mst_shipgraphの propertyを追加
                    for (var a in element) {
                        // mst_shipにはないが mst_shipgraphには存在する要素はとりあえず弾く
                        if (element.hasOwnProperty(a) && mst[element.api_id] && !mst[element.api_id].hasOwnProperty(a)) {
                            mst[element.api_id][a] = element[a];
                        }
                    }
                });
                mst1 = mst;
            })();

            var mst2 = {};
            (function () {
                var j1 = sd.api_data.api_mst_slotitem || [];

                var mst = {};
                j1.forEach(function (element, index, array) {
                    mst[element.api_id] = element;
                });
                mst2 = mst;
            })();

            var db = _kancolle.master.db;
            if (db) {
                var tx = db.transaction([ "Ship", "Equipment" ], "readwrite");
                var o1 = tx.objectStore("Ship");
                for (var a in mst1) {
                    if (mst1.hasOwnProperty(a)) {
                        o1.put(mst1[a]);
                    }
                }
                var o2 = tx.objectStore("Equipment");
                for (var a in mst2) {
                    if (mst2.hasOwnProperty(a)) {
                        o2.put(mst2[a]);
                    }
                }
                //console.log("*** updated master db");
            }

            _kancolle.master.ship = mst1;
            _kancolle.master.equipment = mst2;
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

            var usr1 = {};
            (function () {
                // slot_itemの api_dataはそのままアイテムデータの配列になってる
                var j1 = sd.api_data || [];
    
                var mst = _kancolle.master.equipment;
                var tmp = {};
                j1.forEach(function (element, index, array) {
                    var d = mst && mst[element.api_slotitem_id];
                    if (d) {
                        element.api_slotitem_name = d.api_name;
                        // 厳密には api_type[3]がアイコンの種類を指してる
                        // vo.MasterSlotItemData参照
                        element.api_slotitem_type = d.api_type;
                    }
                    tmp[element.api_id] = element;
                });
                usr1 = tmp;
            })();

            _kancolle.user.equipment = usr1;
        });
    });

    function _updatePanel() {
        if (window.postMessageToPanel) {
            window.postMessageToPanel({ type: "update" });
        }
    }

    function _updateDeck(deck) {
        var tmp = {};
        deck.forEach(function (element, index, array) {
            var d = {};
            d.name = element.api_name;
            d.ship = [];
            element.api_ship.forEach(function (_element) {
                if (_element > 0) {
                    d.ship.push(_element);
                }
            });
            if (element.api_mission[2] > 0) {
                d.mission = {};
                d.mission.arrival = element.api_mission[2];
            }
            tmp[element.api_id] = d;
        });
        return tmp;
    }

    function _updateShip(ship) {
        var mst = _kancolle.master.ship;
        var ueq = _kancolle.user.equipment;

        var tmp = {};
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
                    } else {
                        // TODO: 取れてない旨のメッセージを出す?
                    }
                }
            });
            // 補強増設スロット
            // 現状 1つだけど念のため余裕を持たせておく
            element.api_slot2_name = [];
            element.api_slot2_type = [];
            if (element.api_slot_ex > 0) {
                element.api_slot2_name.push(ueq[element.api_slot_ex].api_slotitem_name);
                element.api_slot2_type.push(ueq[element.api_slot_ex].api_slotitem_type[3]);
                //ueq[element.api_slot_ex].api_owner = element.api_ship_id;
            }
            tmp[element.api_id] = element;
        });
        return tmp;
    }

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

            _kancolle.user.ship = _updateShip(sd.api_data.api_ship || []) || _kancolle.user.ship;
            _kancolle.user.deck = _updateDeck(sd.api_data.api_deck_port || []) || _kancolle.user.deck;
            _updatePanel();
        });
    });

    var kAPIMemberShip2 = /^http:\/\/[^\/]+\/kcsapi\/api_get_member\/ship2$/;
    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!kAPIMemberShip2.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var sd = JSON.parse(content.substring(7)) || {};
            _kancolle.user.ship = _updateShip(sd.api_data || []) || _kancolle.user.ship;
            _updatePanel();
        });
    });

    // 改造時, 装備変更時
    var kAPIMemberShip3 = /^http:\/\/[^\/]+\/kcsapi\/api_get_member\/ship3$/;
    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!kAPIMemberShip3.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var sd = JSON.parse(content.substring(7)) || {};
            var ship = _updateShip(sd.api_data.api_ship_data || []);
            for (var a in ship) {
                if (ship.hasOwnProperty(a)) {
                    _kancolle.user.ship[a] = ship[a];
                }
            }
            _kancolle.user.deck = _updateDeck(sd.api_data.api_deck_data || []) || _kancolle.user.deck;
            _updatePanel();
        });
    });

    var kAPIMemberDeck = /^http:\/\/[^\/]+\/kcsapi\/api_get_member\/deck$/;
    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!kAPIMemberDeck.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var sd = JSON.parse(content.substring(7)) || {};
            _kancolle.user.deck = _updateDeck(sd.api_data || []) || _kancolle.user.deck;
            _updatePanel();
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

            var usr1 = _kancolle.user.ship || {};
            var usr2 = _kancolle.user.deck || {};

            // Upsdate ship
            var ship = _updateShip(sd.api_data.api_ship_data || []);
            for (var a in ship) {
                if (ship.hasOwnProperty(a)) {
                    usr1[a] = ship[a];
                }
            }

            // Update deck
            var deck = _updateDeck(sd.api_data.api_deck_data || []);
            for (var a in deck) {
                if (deck.hasOwnProperty(a)) {
                    usr2[a] = deck[a];
                }
            }

            _updatePanel();
        });
    });

/*
    function _addListener(pattern, callback) {
        chrome.devtools.network.onRequestFinished.addListener(function (request) {
            if (!pattern.test(request.request.url)) {
                return;
            }
            request.getContent(function (content) {
                var sd = JSON.parse(content.substring(7)) || {};
                if (!sd || !sd.api_data) {
                    return;
                }
                callback.call(this, sd);
            });
        });
    }
*/
})(window.kancolle);
