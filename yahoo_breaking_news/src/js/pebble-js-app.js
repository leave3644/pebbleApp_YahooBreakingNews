var CHUNKS_LENGTH = 512;

var Utils = {
    serialize: function(obj) {
        var str = [];
        for(var p in obj)
            str.push(encodeURIComponent(p) + "=" + encodeURIComponent(obj[p]));
        return '?'+(str.join("&"));
    },
    http: function(url, params, cb, errcb) {
        var req = new XMLHttpRequest();

        req.open('GET', url+this.serialize(params), true);
        req.onload = function(e) {
            if (req.readyState != 4) return;
            if (req.status == 200 || req.status == 201)
                cb(req.responseText);
            else
                errcb();
        };
        req.send();
    },
    sendQueue: function (queue){
        var index = retries = 0;

        var doo = function() {
            if (!queue[index]) return;

            console.log('sending '+JSON.stringify(queue[index]));
            Pebble.sendAppMessage(queue[index], success, fail);
        };
        var success = function() {
            console.log('Packet sent');
            index += 1;
            retries = 0;
            doo();
        }
        var fail = function () {
            retries += 1;
            if (retries == 3){
                console.log('Packet fails, moving on');
                index += 1;
            }
            doo();
        }
        doo();
    },
    send: function(data) {
        var chunks = Math.ceil(data.length/CHUNKS_LENGTH),
            queue = [];

        for (var i = 0; i < chunks; i++){
            var payload = {summary:data.substring(CHUNKS_LENGTH*i, CHUNKS_LENGTH*(i+1))};
            if (i == 0) payload.start = "yes";
            if (i == chunks-1) payload.end = "yes";

            queue.push(payload);
        }

        Utils.sendQueue(queue);
    }
};

var BreakingNews = {

    items: [],
    sendItems: function(items) {
        var queue = [];
        items.forEach(function(item, i) {
            queue.push({"index": i, "headline": item.headline, "uuid": item.uuid});
        });
        Utils.sendQueue(queue);
    },
    fetch: function() {
        Utils.http('http://abysmaldismal.corp.ir2.yahoo.com:4080/latency/v1/getNews', null, function(res) {
            BreakingNews.items = JSON.parse(res).news;
            localStorage.setItem('items', BreakingNews.items);
            BreakingNews.sendItems(BreakingNews.items);
        }, function() {
            BreakingNews.sendItems(JSON.parse(localStorage.getItem('items')));
        });
    },
    get: function(i) {
        Utils.http('http://japi1.global.media.yahoo.com:4080/content/v1/object', {uuid: BreakingNews.items[i].uuid }, function(res) {
            var json = JSON.parse(res),
                summary = json.headline + (json.summary) ? json.summary : '';
            Utils.send(summary);
        });
    }
};

Pebble.addEventListener("appmessage", function(e) {
    var action = Object.keys(e.payload)[0];
    switch (action) {
        case "get":
            BreakingNews.get(e.payload[action]);
            break;
        case "fetch":
            BreakingNews.fetch();
            break;
        case "readlater":
            BreakingNews.readlater(e.payload[action]);
            break;
    }
});

Pebble.addEventListener("webviewclosed", function (e) {
    if (!e.response) return;
    var payload = JSON.parse(e.response);
});

Pebble.addEventListener("ready", function () {
    setTimeout(BreakingNews.fetch, 200);
});