let gateway = `ws://${window.location.hostname}/ws`;
window.addEventListener("load", onLoad);
let websocket, rcv, name;

let base64String = "";

function imageUploaded() {
    var file = document.querySelector("input[type=file]")["files"][0];

    var reader = new FileReader();
    reader.onload = function () {
        base64String = reader.result;
        imageBase64Stringsep = base64String;
    };
    reader.readAsDataURL(file);
}

function initWebSocket() {
    console.log("Trying to open a WebSocket connection...");
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log("Connection opened");
    sndFirstReq();
    document.getElementsByClassName("time")[0].innerHTML =
        document.getElementsByClassName("time")[1].innerHTML =
            new Date().getHours() +
            ":" +
            (new Date().getMinutes() < 10 ? "0" : "") +
            new Date().getMinutes();
}

function onClose(event) {
    console.log("Connection closed");
    setTimeout(initWebSocket, 2000);
}

function scrollLast() {
    console.log("scrol");
    let objDiv = document.getElementById("conv");
    objDiv.scrollTop = objDiv.scrollHeight;
}

function sndFirstReq() {
    name = prompt("Enter Your Name (Visible to others)");
    let time =
        new Date().getHours() +
        ":" +
        (new Date().getMinutes() < 10 ? "0" : "") +
        new Date().getMinutes();
    const info = {
        name,
        time,
    };
    console.log(info);
    websocket.send(JSON.stringify(info));
}

function onMessage(event) {
    rcv = JSON.parse(event.data);
    console.log(rcv);
    if (rcv.name && rcv.name != name && rcv.msg && rcv.time) {
        if (rcv.time == "NA")
            rcv.time =
                new Date().getHours() +
                ":" +
                (new Date().getMinutes() < 10 ? "0" : "") +
                new Date().getMinutes();
        if (!rcv.img) {
            document.getElementById("conv").innerHTML +=
                "<div class='text' id='left'><div class='name'>" +
                rcv.name +
                "</div><div class='txt'>" +
                rcv.msg +
                "</div><div class='time'>" +
                rcv.time +
                "</div></div>";
        } else {
            document.getElementById(
                "conv"
            ).innerHTML += `<div class='text' id='left'>
                <div class='name'>${rcv.name}</div>
                <div class='txt'>${rcv.msg}</div>
                <img src=${rcv.img} />
                <div class='time'>${rcv.time}</div>
                </div>`;
        }
    } else if (rcv.name && rcv.name != name && !rcv.msg && rcv.time) {
        document.getElementById("conv").innerHTML +=
            "<div class='join'>" +
            rcv.name +
            " joined the chat at " +
            rcv.time +
            "</div>";
    } else if (!rcv.name && rcv.msg && !rcv.time) {
        document.getElementById(
            "conv"
        ).innerHTML += `<div class='join'>${rcv.msg} are(is) in the chat!!</div>`;
    } else if (rcv.name && rcv.name != name && !rcv.msg) {
        let time =
            new Date().getHours() +
            ":" +
            (new Date().getMinutes() < 10 ? "0" : "") +
            new Date().getMinutes();
        document.getElementById("conv").innerHTML +=
            "<div class='join leave'>" +
            rcv.name +
            " left the chat at " +
            time +
            "</div>";
    }
    setTimeout(scrollLast, 500);
}

function onLoad(event) {
    initWebSocket();
    initButton();
}

function initButton() {
    document.getElementById("in").addEventListener("keypress", (event) => {
        if (event.key == "Enter") {
            document.getElementById("send").click();
        }
    });
}

function sndMsg() {
    const info = {
        msg: document.getElementById("in").value,
        time: "",
        img: "",
    };
    info.time =
        new Date().getHours() +
        ":" +
        (new Date().getMinutes() < 10 ? "0" : "") +
        new Date().getMinutes();

    document.getElementById("in").value = "";
    if (document.getElementsByTagName("input")[0].value) {
        info.img = base64String;
        document.getElementById(
            "conv"
        ).innerHTML += `<div class='text' id='right'>
                <div class='txt'>${info.msg}</div>
                <img src=${base64String} />
                <div class='time'>${info.time}</div>
                </div>`;
    } else if (info.msg) {
        document.getElementById(
            "conv"
        ).innerHTML += `<div class='text' id='right'>
            <div class='txt'>${info.msg}</div>
            <div class='time'>${info.time}</div>
            </div>`;
    }
    console.log(info);
    websocket.send(JSON.stringify(info));
    setTimeout(scrollLast, 500);
}
