/*
 ------------------------------------------
 Yahoo Breaking News for Pebble SmartWatch!
 ------------------------------------------
 */

 /*Read and write helper*/
var reading = function(key, value) {

    window.localStorage.setItem(key,value);
};

var writing = function(key, defValue) {

     if(window.localStorage.getItem(key) === null){
        return defValue;
     } else {
        return window.localStorage.getItem(key);
     }

};

function reFetch() {
    current_int = 0;
    // get the current news with title and summary
    var category = reading("category", "breakingNews");
    news = download(category);


}

/*Basic event listener*/

Pebble.addEventListener("ready",
    function(e) {
        console.log("Hello world! - Sent from your javascript application.");
        reFetch();
    }
);

Pebble.addEventListener("webviewclosed",
    function(e) {
        //get JSON data
        var config = JSON.parse(decodeURIComponent(e.response));
        console.log(JSO.stringify(config));

        //save to the local storage
        writing("breaking news number is ", config.numbers);

        //send the data to watch
        Pebble.sendAppMessage(
            {"KEY_DETAIL_SIZE": config.detail_size}
            );
        }
    );

/*get the http request and return to xml response text*/
var HTTPFetch = function(url, type, callback){
    var request = new XMLHttpRequest();
    request.onload = function() {
        callback(this.responseText);
    };
    request.open(type, url);
    request.send();
}

var news;
var current_title = 0;
var current_sum = 0;
var numbers = 0;


/* Breaking New data*/

function BreakingNews(title, summary) {
    this.summary = summary;
    this.title = title;
};

/*This will fetch the latest feed*/
var fetchFeed = function(text) {
    var items = [];
    while(text.indexOf("<title>") > 0 ) {
        //Get the current title
        var title = text.substring(text.indexOf("<title>") + "<title>".length);
        title = title.substring(0, title.indexOf("</title>"));
        text = text.substring(text.indexOf("</title>")) + "</title>".length;

        //Get the current summary
        var sum = text.substring(text.indexOf("<summary>") + "<summary>".length);
        sum = sum.substring(0, title.indexOf("</summary>"));

        //add the current the title and summary
        items.push(new BreakingNews(title, sum));
        text = text.substring(text.indexOf("</summary>") + "</summary>".length);
        
    }

    console.log("fetchFeed(): Loading" + items.length + " items ");
    return items;

};

//download the data from server
var download = function(category){
    var url; // load the url string
    console.log("getting the data from url");
    if(category == "latest") {
        url //update url address;
        console.log("choose the latest one");
    }
    else{
        console.log("didn't choose any option");
    }
    HTTPFetch(url, "GET", 
        function(text) {
            console.log("Got the Breaking News!");
            text = text.substring(text.indexOf("<item>") + "<item>".length);
            news = fetchFeed(text);

            //get the latest 10 breaking news
            numbers = parseInt(reading("number","10"));
            console.log("reading(): number of news is " + numbers);

            Pebble.sendAppMessage(
                {"KEY_QUANTITY" : numbers},
                function(e) {
                    console.log("Numbers" + numbers + " sent, start loading.");
                    //sendNextNews();
                },
                function(e) {
                    console.log("Reading news number failed :( ");;
                }
                );
            }
        );
    console.log("Reading request sent");
};

