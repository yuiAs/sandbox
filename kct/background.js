(function (_window) {
    chrome.runtime.onConnect.addListener(function (port) {
        if (port.name == "registerFilters") {
            chrome.webRequest.onBeforeRequest.addListener(function (details) {
                if (port && details.requestBody) {
                    port.postMessage({ url: details.url, data: details.requestBody.formData });
                }
            }, { urls: [ "http://*/kcsapi/api_req_hensei/change" ] }, [ "blocking", "requestBody" ]);
            port.onDisconnect.addListener(function () {
                // TODO: onBeforeRequest.removeListenerやっておいたほうが無駄に listener残らなくていいかも
                port = void 0;
            });
        }
    });
})(window || this);

(function (_window) {
    chrome.contextMenus.create({
        type: "checkbox",
        id: "MenuKCTLogs",
        title: "Logs",
        checked: false,
        documentUrlPatterns: [ "http://www.dmm.com/netgame/social/-/gadgets/=/app_id=854854/" ],
    });
    chrome.contextMenus.create({
        type: "checkbox",
        id: "MenuKCTGrayscaled",
        title: "Grayscaled",
        checked: false,
        documentUrlPatterns: [ "http://www.dmm.com/netgame/social/-/gadgets/=/app_id=854854/" ],
    });
    chrome.contextMenus.onClicked.addListener(function (info, tab) {
        if (tab) {
            chrome.tabs.sendMessage(tab.id, info);
        }
    });
})(window || this);
