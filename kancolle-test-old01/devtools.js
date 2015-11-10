
var _global = this || window;

_global.port1 = null;
_global.port2 = null;

_global.masterShips = {};
_global.userShips = {};
_global.userFleet = [];
_global.userShipsCount = 0;

_global.update = function (time, caller) {
    console.log('update', caller, time);

    var deck = _global.userFleet || [];
    //console.log(deck);

    var f1 = function (name, mission) {
        var s = [];
        //s.push(name);
        //s.push(mission[0].toString(10) + '-' + mission[1].toString(10));
        s.push((new Date(mission[2])).toLocaleString('en-US', { hour12: false }));
        return s.join(' ');
    };
    var f2 = function (fleet) {
        var s = [];
        for (var i = 0; i < fleet.length; ++i) {
            var f = fleet[i];
            if ((f >= 0) && _global.userShips[f]) {
                var u = _global.userShips[f];
                var m = _global.masterShips && _global.masterShips[u.id];
                var t = (m && m.api_name) || '';
                t += u.hp2 + '/' + u.hp1 + '/' + u.cond2;
                s.push(t);
            }
        }
        return s;
    };

    var r = [];
    for (var i = 0; i < deck.length; ++i) {
        var d = deck[i];
        var t = [];
        t.push((i + 101).toString(10).substring(1));
        if (d.api_mission[2]) {
            t.push(f1(d.api_name, d.api_mission));
        } else {
        }
        Array.prototype.push.apply(t, f2(d.api_ship || []));
        r.push(t.join(' '));
    }

    if (_global.port2) {
        _global.port2.postMessage({
            data: r,
            date: time,
        });
    }
};


// Connect to port
(function (_this) {
    _this.port1 = chrome.runtime.connect({ name: '054D0C05-D1B5-4CF8-816A-AB544C715709' });
    _this.port2 = chrome.runtime.connect({ name: 'CFF5E736-C7B1-43D9-9236-E1A581544B1E' });

    if (_this.port1) {
        _this.port1.onMessage.addListener(function (msg) {
            if (msg && msg.svdata) {
                var f = _this.userFleet[msg.svdata.api_id - 1] || [];
                var s = f.api_ship || [];
                var i = msg.svdata.api_ship_idx|0;
                if (i == -1) {
                    for (var j = 1; j < s.length; ++j) {
                        s[j] = -1;
                    }
                } else if (i < s.length) {
                    var v = msg.svdata.api_ship_id|0;
                    // -1の場合は削除になってるので詰める
                    if (v < 0) {
                        s.splice(i, 1);
                        s.push(v);
                    } else {
                        s[i] = v;
                    }
                }
                _this.update(Date.now(), 'connect1');
            }
        });
    }
})(_global);


// Event handlers for chrome.devtools.network.onRequestFinished
(function (_this) {
    var API_ROOT_START2 = /^http:\/\/[^\/]+\/kcsapi\/api_start2$/;
    var API_PORT_PORT = /^http:\/\/[^\/]+\/kcsapi\/api_port\/port$/;
    var API_MEMBER_SHIP2 = /^http:\/\/[^\/]+\/kcsapi\/api_get_member\/ship2$/;
    var API_MEMBER_DECK = /^http:\/\/[^\/]+\/kcsapi\/api_get_member\/deck$/;

    var updateShipsMaster = function (ships) {
        for (var i = 0; i < ships.length; ++i) {
            var m = ships[i];
            var c = _this.userShips[m.api_id];
            var p = {
                id: m.api_ship_id,
                cond1: c ? c.cond2 : m.api_cond,
                cond2: m.api_cond,
                hp1: m.api_maxhp,
                hp2: m.api_nowhp,
            };
            _this.userShips[m.api_id] = p;
        }
    };

    var updateDeck = function (deck) {
        for (var i = 0; i < deck.length; ++i) {
            _this.userFleet[i] = deck[i];
        }
    };

    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!API_ROOT_START2.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var svdata = JSON.parse(content.substring(7)) || {};
            svdata.api_data = svdata.api_data || {};
            svdata.api_data.api_mst_ship = svdata.api_data.api_mst_ship || [];

            // 超非効率だけどここでやってれば後が楽
            for (var i = 0; i < svdata.api_data.api_mst_ship.length; ++i) {
                var m = svdata.api_data.api_mst_ship[i];
                if (m) {
                    _this.masterShips[m.api_id] = m;
                }
            }
        });
    });

    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!API_PORT_PORT.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var svdata = JSON.parse(content.substring(7)) || {};
            svdata.api_data = svdata.api_data || {};

            // Update user's master
            updateShipsMaster(svdata.api_data.api_ship || []);
            // Update number of ships
            _this.userShipsCount = (svdata.api_data.api_ship && svdata.api_data.api_ship.length) || 0;
            // Update deck
            updateDeck(svdata.api_data.api_deck_port || []);

            _this.update(Date.now(), 'port');
        });
    });

    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!API_MEMBER_SHIP2.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var svdata = JSON.parse(content.substring(7)) || {};
            svdata.api_data = svdata.api_data || {};

            // Update user's master
            updateShipsMaster(svdata.api_data || []);

            _this.update(Date.now(), 'ship2');
        });
    });

    chrome.devtools.network.onRequestFinished.addListener(function (request) {
        if (!API_MEMBER_DECK.test(request.request.url)) {
            return;
        }
        request.getContent(function (content) {
            var svdata = JSON.parse(content.substring(7)) || {};
            svdata.api_data = svdata.api_data || {};

            // Update deck
            updateDeck(svdata.api_data || []);

            _this.update(Date.now(), 'deck');
        });
    });

})(_global);
