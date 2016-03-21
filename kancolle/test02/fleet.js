var _local = window || this;

document.addEventListener("DOMContentLoaded", function () {
    WinJS.UI.processAll().done(function () {
        _initControls();
    });
});

window.addEventListener("message", function (event) {
    //console.log("message", event);
    switch (event.data.type) {
        case "setObject":
        case "update":
            _assignDeckList(document);
            break;
    }
});

function _initControls() {
    var tp = document.querySelector("tbody.table-lvitem-template");
    document.querySelector("tbody.table-repeater").winControl.template = function (data) {
        var tmpl = tp.cloneNode(true);
        tmpl.querySelector(".lvhead-name").textContent = data.name;
        if (data.mission) {
            tmpl.querySelector(".lvhead-time").textContent = (new Date(data.mission.arrival)).toLocaleString('en-US', { hour12: false });
        }
        data.ship.forEach(function (element, index, array) {
            var ship = _local.kancolle.user.ship[element] || [];
            var node = tmpl.querySelector(".lvitem").cloneNode(true);
            if (ship) {
                node.querySelector(".lvitem-name").textContent = ship.api_ship_name;
                node.querySelector(".lvitem-lv").textContent = ship.api_lv;
                //node.querySelector(".lvitem-hp").textContent = ship.api_nowhp + "/" + ship.api_maxhp;
                var elem3 = node.querySelector(".lvitem-hp");
                if (elem3) {
                    elem3.textContent = ship.api_nowhp + "/" + ship.api_maxhp;
                    var rate = 100.0 * ship.api_nowhp / ship.api_maxhp;
                    if (rate > 75) {
                    } else {
                        if (rate > 50) {
                            elem3.classList.add("color-rate1");
                        } else {
                            if (rate > 25) {
                                elem3.classList.add("color-rate2");
                            } else {
                                elem3.classList.add("color-rate3");
                            }
                        }
                    }
                }
                //node.querySelector(".lvitem-x1").textContent = ship.api_slot_name.join(",");
                var elem1 = node.querySelector(".lvitem-x1");
                ship.api_slot_type.forEach(function (_element, _index) {
                    // icons.swf 2.4.0
                    // 実際は存在しない slot_type=0のアイコンに slot_type=1のアイコンが仮置きされてる
                    var v = (_element - 1) * 2 + 70;
                    var n = document.createElement("img");
                    n.src = "/assets/icons/" + v + ".png";
                    n.alt = ship.api_slot_name[_index];
                    n.title = ship.api_slot_name[_index];
                    elem1.appendChild(n);
                });
                // 補強増設スロット用
                // TODO: 通常のスロットと同じ処理なので共通化
                var elem2 = node.querySelector(".lvitem-x2");
                ship.api_slot2_type.forEach(function (_element, _index) {
                    var v = (_element - 1) * 2 + 70;
                    var n = document.createElement("img");
                    n.src = "/assets/icons/" + v + ".png";
                    n.alt = ship.api_slot2_name[_index];
                    n.title = ship.api_slot2_name[_index];
                    elem2.appendChild(n);
                });
                node.querySelector(".lvitem-condition").textContent = ship.api_cond;
            } else {
                node.querySelector(".lvitem-name").textContent = element;
            }
            tmpl.appendChild(node);
        });
        // 最初にある tr.lvitemを削除
        tmpl.querySelector(".lvitem").remove();
        tmpl.classList.remove("table-lvitem-template");
        return tmpl;
    };
    _assignDeckList(document);
}

function _assignDeckList(_document) {
    var ls = new WinJS.Binding.List();
    // user.deckがある = user.shipもある前提
    var deck = _local.kancolle.user && _local.kancolle.user.deck;
    if (deck) {
        for (var a in deck) {
            if (deck.hasOwnProperty(a)) {
                ls.push(deck[a]);
            }
        }
    }
    _document.querySelector(".table-repeater").winControl.data = ls;
}
