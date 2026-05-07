self.Module = {
    onRuntimeInitialized: function () {
        onWasmLoaded();
    }
};

self.importScripts("common.js");
self.importScripts("libffmpeg.js");

function Decoder() {
    this.logger = new Logger("Decoder");
    this.wasmLoaded = false;
    this.tmpReqQue = [];
    this.cacheBuffer = null;
    this.videoCallback = null;
    this.audioCallback = null;
}

Decoder.prototype.initDecoder = function (chunkSize) {
    this.logger.logInfo("Create cacha buffer " + chunkSize);
    this.cacheBuffer = Module._malloc(chunkSize);
    var objData = {
        t: kInitDecoderRsp,
        e: 0
    };
    self.postMessage(objData);
};

Decoder.prototype.uninitDecoder = function () {
    this.logger.logInfo("Destroy cacha buffer.");
    if (this.cacheBuffer != null) {
        Module._free(this.cacheBuffer);
        this.cacheBuffer = null;
    }
};

Decoder.prototype.openDecoder = function () {
    let objData;
    const ret = Module._openDecoder(this.videoCallback, this.audioCallback, 0);
    if (ret === 0) {

        objData = {
            t: kOpenDecoderRsp,
            e: ret,
            v: {
                d: -1,
                p: 0,
                w: 0,
                h: 0
            },
            a: {
                f: 1, // 16bit
                c: 1,
                r: 8000
            }
        };
        self.postMessage(objData);
    } else {
        objData = {
            t: kOpenDecoderRsp,
            e: ret
        };
        self.postMessage(objData);
    }
};

Decoder.prototype.closeDecoder = function () {

    Module._closeDecoder();
    const objData = {
        t: kCloseDecoderRsp,
        e: 0
    };
    self.postMessage(objData);
};


Decoder.prototype.sendData = function (data) {
    var typedArray = new Uint8Array(data);
    Module.HEAPU8.set(typedArray, this.cacheBuffer);
    let r = Module._decodeData(this.cacheBuffer, typedArray.length);
    // console.log("decode result %o ", r);
};


Decoder.prototype.processReq = function (req) {
    //this.logger.logInfo("processReq " + req.t + ".");
    switch (req.t) {
        case kInitDecoderReq:
            this.initDecoder(req.c);
            break;
        case kUninitDecoderReq:
            this.uninitDecoder();
            break;
        case kOpenDecoderReq:
            this.openDecoder();
            break;
        case kCloseDecoderReq:
            this.closeDecoder();
            break;
        case kFeedDataReq:
            this.sendData(req.d);
            break;
        default:
            this.logger.logError("Unsupport messsage " + req.t);
    }
};

Decoder.prototype.cacheReq = function (req) {
    if (req) {
        this.tmpReqQue.push(req);
    }
};

Decoder.prototype.onWasmLoaded = function () {
    this.logger.logInfo("Wasm loaded.");
    this.wasmLoaded = true;

    this.videoCallback = Module.addFunction(function (buff, size, width, height, timestamp) {
        const outArray = Module.HEAPU8.subarray(buff, buff + size);
        const data = new Uint8Array(outArray);
        const objData = {
            t: kVideoFrame,
            s: timestamp,
            w: width,
            h: height,
            d: data
        };
        self.postMessage(objData, [objData.d.buffer]);
    }, 'viiiid');

    this.audioCallback = Module.addFunction(function (buff, size, timestamp) {
        const outArray = Module.HEAPU8.subarray(buff, buff + size);
        const data = new Uint8Array(outArray);
        const objData = {
            t: kAudioFrame,
            s: timestamp,
            d: data
        };
        self.postMessage(objData, [objData.d.buffer]);
    }, 'viid');

    while (this.tmpReqQue.length > 0) {
        const req = this.tmpReqQue.shift();
        this.processReq(req);
    }
};

self.decoder = new Decoder;

self.onmessage = function (evt) {
    if (!self.decoder) {
        console.log("[ER] Decoder not initialized!");
        return;
    }

    const req = evt.data;
    if (!self.decoder.wasmLoaded) {
        self.decoder.cacheReq(req);
        self.decoder.logger.logInfo("Temp cache req " + req.t + ".");
        return;
    }

    self.decoder.processReq(req);
};

function onWasmLoaded() {
    if (self.decoder) {
        self.decoder.onWasmLoaded();
    } else {
        console.log("[ER] No decoder!");
    }
}
