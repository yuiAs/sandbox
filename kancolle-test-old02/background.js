(function (_this) {
    chrome.runtime.onConnect.addListener(function (port) {
        if (port.name == "registerFilters") {
            chrome.webRequest.onBeforeRequest.addListener(function (details) {
                //console.log("onBeforeRequest", details);
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
})(this || window);
