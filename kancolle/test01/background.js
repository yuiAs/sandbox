
(function () {
    var port1;
    var port2;

    chrome.webRequest.onBeforeRequest.addListener(function (details) {
        if (details.requestBody) {
            if (port1) {
                port1.postMessage({ svdata: details.requestBody.formData });
            }
        }
    }, { urls: [ 'http://*/kcsapi/api_req_hensei/change' ] }, [ 'blocking', 'requestBody' ]);

    chrome.runtime.onConnect.addListener(function (_port) {
        if (_port) {
            switch (_port.name) {
                case '054D0C05-D1B5-4CF8-816A-AB544C715709':
                    port1 = _port;
                    port1.onDisconnect.addListener(function () {
                        port1 = null;
                    });
                    break;
                case '8753CCA5-3137-4878-A832-F426FA4346D9':
                    port2 = _port;
                    port2.onDisconnect.addListener(function () {
                        port2 = null;
                    });
                    break;
                case 'CFF5E736-C7B1-43D9-9236-E1A581544B1E':
                    _port.onMessage.addListener(function (msg) {
                        if (port2) {
                            port2.postMessage(msg);
                        }
                    });
                    break;
            }
        }
    });
})();
