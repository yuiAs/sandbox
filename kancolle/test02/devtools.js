(function (_this) {
    _this.kancolle = _this.kancolle || {};
    _this.kancolle.master = _this.kancolle.master || {};

    _this.postMessageToPanel = null;


    // まず現在の URIを見て，異なるなら Listenerを追加する
    var kBaseURI = /^http:\/\/www\.dmm\.com\/netgame\/social\/-\/gadgets\/=\/app_id=854854\/$/;
    chrome.devtools.inspectedWindow.eval("document.baseURI", function (result, exceptionInfo) {
        if (kBaseURI.test(result)) {
            _initExtension();
        } else {
            chrome.devtools.network.onNavigated.addListener(function _checker(url) {
                if (kBaseURI.test(url)) {
                    chrome.devtools.network.onNavigated.removeListener(_checker);
                    _initExtension();
                }
            });
        }
    });

    function _initExtension() {
        // DB接続
        // upgradeneededのあと successが呼ばれるっぽいので upgradeneededでは object storeの作成だけ
        _this.kancolle.master.db = null;
        (function (_kancolle) {
            // MasterData
            var req1 = window.indexedDB.open("MasterData", 1);
            req1.onupgradeneeded = function (event) {
                //console.log("db open (upgrade)", req1);
                var db = event.target.result;
                db.createObjectStore("Ship", { keyPath: "api_id" });
                db.createObjectStore("Equipment", { keyPath: "api_id" });
                event.target.transaction.oncomplete = function() {
                    //console.log("*** upgraded");
                };
            };
            req1.onsuccess = function (event) {
                //console.log("db open (success)", req1);
                _loadMasterDataFromDatabase(event.target.result);
                _kancolle.master.db = event.target.result;
            };
        })(_this.kancolle);

        // Panelを生成
        chrome.devtools.panels.create("Kancolle", "", "/fleet.html", function (panel) {
            panel.onShown.addListener(function onShown(_window) {
                _window.kancolle = _this.kancolle;
                // 生成した Panelとのメッセージングに使う関数を作成
                _this.postMessageToPanel = function (message) {
                    _window.postMessage(message, _window.document.origin);
                };
                _this.postMessageToPanel({ type: "setObject" });
            });
        });

        // 一部のリクエストの内容を見ないと何をやっているかわからない APIの監視
        _this.backgroundPort = chrome.runtime.connect({ name: "registerFilters" });
        _this.backgroundPort.onMessage.addListener(function (message, sender, sendResponse) {
            if (message.data && _this.kancolle.user.deck) {
                var sd = message.data;
                var usd = _this.kancolle.user.deck || {};
                if (sd.api_ship_idx < 0) {
                    // api_ship_idx==-1の場合は旗艦を残して全解除
                    if (usd[sd.api_id]) {
                        usd[sd.api_id].ship.splice(1, usd[sd.api_id].ship.length - 1);
                    }
                } else {
                    if (usd[sd.api_id]) {
                        // api_ship_id==-1の場合は単体解除
                        if (sd.api_ship_id == -1) {
                            usd[sd.api_id].ship.splice(sd.api_ship_idx, 1);
                        } else {
                            var i = usd[sd.api_id].ship.indexOf(sd.api_ship_id|0);
                            if (i == -1) {
                                // 追加，差し替え
                                usd[sd.api_id].ship.splice(sd.api_ship_idx, 1, sd.api_ship_id|0);
                            } else {
                                // 入れ替え
                                usd[sd.api_id].ship[sd.api_ship_idx] = [sd.api_ship_id|0, usd[sd.api_id].ship[i] = usd[sd.api_id].ship[sd.api_ship_idx]][0];
                            }
                        }
                    }
                }
                if (_this.postMessageToPanel) {
                    _this.postMessageToPanel({ type: "update" });
                }
            }
        });
    }

    // TODO: 何らかの webRequestをトリガーに無ければロードする
    function _loadMasterDataFromDatabase(db) {
        var tx = db.transaction([ "Ship", "Equipment" ], "readonly");
        var mst1 = {};
        tx.objectStore("Ship").openCursor().onsuccess = function (event) {
             var cur = event.target.result;
             if (cur) {
                 mst1[cur.value.api_id] = cur.value;
                 cur.continue();
             } else {
                 _this.kancolle.master.ship = mst1;
             }
        };
        var mst2 = {};
        tx.objectStore("Equipment").openCursor().onsuccess = function (event) {
             var cur = event.target.result;
             if (cur) {
                 mst2[cur.value.api_id] = cur.value;
                 cur.continue();
             } else {
                 _this.kancolle.master.equipment = mst2;
             }
        };
    }
})(window);
