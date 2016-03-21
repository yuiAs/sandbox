(function (_window) {
    _window.kct = _window.kct || {};

    var root = document.querySelector("#area-game") || document.body || document.documentElement;
    var node = document.createElement("div");
    node.classList.add("kct-body");
    var logs = document.createElement("div");
    logs.classList.add("kct-logs");
    node.appendChild(logs);
    root.appendChild(node);
    root = void 0;

    _window.kct.logs = logs;
    _window.kct.menu = {};
    _window.kct.menu["MenuKCTLogs"] = function (_bool) {
        if (node.classList.contains("is-invisible") ^ !_bool) {
            node.classList.toggle("is-invisible");
        }
    };
    _window.kct.menu["MenuKCTGrayscaled"] = function (_bool) {
        var body = document.body || document.documentElement;
        if (body && (body.classList.contains("is-grayscaled") ^ _bool)) {
            body.classList.toggle("is-grayscaled");
        }
    };
    _window.kct.menu["MenuKCTLogs"](false);

    chrome.runtime.onMessage.addListener(function (message, sender, sendResponse) {
        _window.kct.menu[message.menuItemId](message.checked);
    });
})(window || this);

(function (_window) {
    console.log(Date.now() + ": kct");

    function updateDeck(deck) {
        var cf = document.createDocumentFragment();

        for (var a in deck) {
            if (deck.hasOwnProperty(a)) {
                var f = deck[a];

                var d1 = document.createElement("div");
                if (f.mission) {
                    d1.textContent = "----- " + (new Date(f.mission.arrival)).toLocaleString('en-US', { hour12: false });
                } else {
                    d1.textContent = "----- ";
                }
                cf.appendChild(d1);

                f.ship2.forEach(function (element, index, array) {
                    var d2 = document.createElement("div");

                    // name
                    var s1 = document.createElement("span");
                    s1.classList.add("name");
                    s1.textContent = element.api_ship_name;
                    d2.appendChild(s1);

                    // status
                    var s2 = document.createElement("span");
                    var t1 = [];
                    t1.push(("  " + element.api_lv).slice(-3));
                    t1.push(("  " + element.api_cond).slice(-3));
                    //t1.push(("    " + element.api_nowhp + "/" + element.api_maxhp).slice(-7));
                    //s2.textContent = t1.join(" ");
                    s2.appendChild(document.createTextNode(t1.join(" ")));
                    var s4 = document.createElement("span");
                    var rate = 100.0 * element.api_nowhp / element.api_maxhp;
                    if (rate > 75) {
                    } else {
                        if (rate > 50) {
                            s4.classList.add("rate-color1");
                        } else {
                            if (rate > 25) {
                                s4.classList.add("rate-color2");
                            } else {
                                s4.classList.add("rate-color3");
                            }
                        }
                    }
                    s4.textContent = ("     " + element.api_nowhp + "/" + element.api_maxhp).slice(-8);
                    s2.appendChild(s4);
                    d2.appendChild(s2);

                    // equipments
                    var s3 = document.createElement("span");
                    var t2 = [];
                    element.api_slot_name.forEach(function (_element, _index, _array) {
                        t2.push(_element);
                    });
                    element.api_slot2_name.forEach(function (_element, _index, _array) {
                        t2.push(_element);
                    });
                    s3.textContent = t2.join(",");
                    d2.appendChild(s3);

                    cf.appendChild(d2);
                });
            }
        }

        while (_window.kct.logs.firstChild) {
            _window.kct.logs.removeChild(_window.kct.logs.firstChild);
        }
        _window.kct.logs.appendChild(cf);
    }

    chrome.storage.onChanged.addListener(function (changes, areaName) {
        if (areaName != "local") {
            return;
        }
        //console.log(changes);
        if (changes && changes["deck"]) {
            updateDeck(changes["deck"].newValue);
        }
    });
})(window || this);
