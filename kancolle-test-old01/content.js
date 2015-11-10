
// テキスト出力用の要素生成
var privNode = document.createElement('pre');
privNode.id = 'kancolle-test-logs';

(function (element) {
    var elem1 = document.createElement('div');
    elem1.id = 'kancolle-test-root';
    elem1.appendChild(element);
    (document.getElementById('area-game') || document.body || document.documentElement).appendChild(elem1);

    var elem3 = document.createElement('li');
    elem3.textContent = 'State';
    elem3.changeViewState = function () {
        var elem = document.querySelector('#kancolle-test-root') || document.createElement('div');
        if (elem.style.visibility && (elem.style.visibility != 'hidden')) {
            elem.style.visibility = 'hidden';
        } else {
            elem.style.visibility = 'visible';
        }
    };
    elem3.addEventListener('click', function () {
        this.changeViewState();
    });
    elem3.changeViewState();

    var elem4 = document.createElement('li');
    elem4.textContent = 'Grey';
    elem4.changeViewState = function () {
        (document.querySelector('#game_frame') || document.createElement('div')).classList.toggle('game_frame_filter');
    };
    elem4.addEventListener('click', function () {
        this.changeViewState();
    });

    // Flash内でのポインタ判定がわけわからないことになるっぽいので保留
/*
    var elem5 = document.createElement('li');
    elem5.textContent = 'Scale';
    elem5.changeViewState = function () {
        (document.querySelector('#game_frame') || document.createElement('div')).classList.toggle('game_frame_scaler');
    };
    elem5.addEventListener('click', function () {
        this.changeViewState();
    });
*/
    var elem2 = document.querySelector('div#dmm_ntgnavi .navi_right') || document.createElement('ul');
    while (elem2.firstChild) {
        elem2.removeChild(elem2.firstChild);
    }
    elem2.appendChild(elem3);
    elem2.appendChild(elem4);
    //elem2.appendChild(elem5);
})(privNode);

(function (element) {
    var port = chrome.runtime.connect({ name: '8753CCA5-3137-4878-A832-F426FA4346D9' });
    if (port) {
        var onMessage = function (msg) {
            console.assert(msg);
            var r = [];
            r.push((new Date(msg.date)).toLocaleString('en-US', { hour12: false }));
            r.push(msg.data.join('\n'));
            r.push('\n');
            element.textContent = r.join('\n');
        };
        port.onMessage.addListener(onMessage);

        port.onDisconnect.addListener(function () {
            console.log('Disconnected from 8753CCA5-3137-4878-A832-F426FA4346D9');
            port = chrome.runtime.connect({ name: '8753CCA5-3137-4878-A832-F426FA4346D9' });
            if (port) {
                port.onMessage.addListener(onMessage);
            }
        });
    }
})(privNode);
