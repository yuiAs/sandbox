(function () {
    console.log("TweetDeckModifier");

    function _code() {
         function _core() {
             var _td = window.TD;
             if (_td === void 0) {
                 console.error("not exists window.TD");
                 return;
             }

             _td.services.TwitterMedia.prototype._large = _td.services.TwitterMedia.prototype.large;
             _td.services.TwitterMedia.prototype.large = function () {
                 switch (this.service) {
                     case "twitter":
                         return this.getTwitterPreviewUrl(":orig");
                 }
                 return this._large();
             }
        }

        setTimeout(function () {
            console.log(window.TD);
            _core();
        }, 1000);
    }


    // 一度ドキュメントノードに追加して即削除
    var elem = document.createElement("script");
    elem.type = "text/javascript";
    elem.innerHTML = "(" + _code.toString() + ")();";
    document.body.appendChild(elem);
    document.body.removeChild(elem);
})();
