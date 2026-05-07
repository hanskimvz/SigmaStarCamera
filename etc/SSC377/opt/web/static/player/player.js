//Player states.
const playerStateIdle = 0;
const playerStatePlaying = 1;
const playerStatePausing = 2;

String.prototype.startWith = function (str) {
    var reg = new RegExp("^" + str);
    return reg.test(this);
};

function FileInfo(url) {
    this.url = url;
    this.chunkSize = 2 * 1024 * 1024;
}

function Player() {
    this.fileInfo = null;
    this.pcmPlayer = null;
    this.canvas = null;
    this.webglPlayer = null;
    this.callback = null;
    this.playerState = playerStateIdle;
    this.isFullscreen = false;
    this.muted = true;
    this.audioEncoding = "";
    this.audioChannels = 0;
    this.audioSampleRate = 0;
    this.frameBuffer = null;
    this.streamPauseParam = null;
    this.logger = new Logger("Player");
    this.initDecodeWorker();
    this.streamSource = null;
}

Player.prototype.initDecodeWorker = function () {
    var self = this;
    this.decodeWorker = new Worker("decoder.js");
    this.decodeWorker.onmessage = function (evt) {
        var objData = evt.data;
        switch (objData.t) {
            case kInitDecoderRsp:
                self.onInitDecoder(objData);
                break;
            case kOpenDecoderRsp:
                self.onOpenDecoder(objData);
                break;
            case kVideoFrame:
                self.onVideoFrame(objData);
                break;
            case kAudioFrame:
                self.onAudioFrame(objData);
                break;
        }
    }
};

Player.prototype.play = function (url, canvas, callback) {
    // this.logger.logInfo("Play " + url + ".");

    let ret = {
        e: 0,
        m: "Success"
    };

    let success = true;
    do {
        if (this.playerState === playerStatePausing) {
            ret = this.resumeStream();
            break;
        }

        if (this.playerState === playerStatePlaying) {
            break;
        }

        if (!url) {
            ret = {
                e: -1,
                m: "Invalid url"
            };
            success = false;
            this.logger.logError("[ER] playVideo error, url empty.");
            break;
        }

        if (!canvas) {
            ret = {
                e: -2,
                m: "Canvas not set"
            };
            success = false;
            this.logger.logError("[ER] playVideo error, canvas empty.");
            break;
        }

        if (!this.decodeWorker) {
            ret = {
                e: -4,
                m: "Decoder not initialized"
            };
            success = false;
            this.logger.logError("[ER] Decoder not initialized.");
            break
        }

        this.fileInfo = new FileInfo(url);
        this.canvas = canvas;
        this.callback = callback;
        this.playerState = playerStatePlaying;
        this.displayLoop();

        //var playCanvasContext = playCanvas.getContext("2d"); //If get 2d, webgl will be disabled.
        this.webglPlayer = new WebGLPlayer(this.canvas, {
            preserveDrawingBuffer: false
        });

        this.createDecoderBuffer();
        this.requestStream(url);

        let self = this
        this.registerVisibilityEvent(function (visible) {
            if (visible) {
                self.resumeStream();
            } else {
                self.pauseStream();
            }
        });

    } while (false);

    return ret;
};

Player.prototype.isPlaying = function () {
    return (this.playerState === playerStatePlaying);
}

Player.prototype.mute = function (muted) {
    this.muted = muted;
}

Player.prototype.pauseStream = function () {
    let ret;
    if (this.playerState !== playerStatePlaying) {
        ret = {
            e: -1,
            m: "Not playing"
        };
        return ret;
    }

    this.streamPauseParam = {
        url: this.fileInfo.url,
        canvas: this.canvas,
        callback: this.callback,
    }

    this.logger.logInfo("Stop in stream pause.");
    this.stop();

    ret = {
        e: 0,
        m: "Success"
    };

    return ret;
}

Player.prototype.resumeStream = function () {
    let ret;
    if (this.playerState !== playerStateIdle || !this.streamPauseParam) {
        ret = {
            e: -1,
            m: "Not pausing"
        };
        return ret;
    }

    this.logger.logInfo("Play in stream resume.");
    this.play(this.streamPauseParam.url,
        this.streamPauseParam.canvas,
        this.streamPauseParam.callback);
    this.streamPauseParam = null;

    ret = {
        e: 0,
        m: "Success"
    };

    return ret;
}


Player.prototype.stop = function () {
    const ret = {
        e: -1,
        m: "Not playing"
    };
    // this.logger.logInfo("Stop.");
    if (this.playerState === playerStateIdle) {
        return ret;
    }


    this.fileInfo = null;
    this.canvas = null;
    this.webglPlayer = null;
    this.callback = null;
    this.playerState = playerStateIdle;
    this.isFullscreen = false;
    this.muted = true;
    this.frameBuffer = null;

    this.releaseStream();

    if (this.pcmPlayer) {
        this.pcmPlayer.destroy();
        this.pcmPlayer = null;
        this.logger.logInfo("Pcm player released.");
    }


    this.logger.logInfo("Closing decoder.");
    this.decodeWorker.postMessage({
        t: kCloseDecoderReq
    });


    this.logger.logInfo("Uniniting decoder.");
    this.decodeWorker.postMessage({
        t: kUninitDecoderReq
    });

    return ret;
};


Player.prototype.fullscreen = function () {
    if (this.webglPlayer) {
        this.isFullscreen ?
            this.webglPlayer.exitfullscreen() :
            this.webglPlayer.fullscreen();
        this.isFullscreen = !this.isFullscreen;
    }
};

Player.prototype.getState = function () {
    return this.playerState;
};


Player.prototype.createDecoderBuffer = function () {
    if (this.playerState === playerStateIdle) {
        return;
    }

    // this.logger.logInfo("Initializing decoder chunk size.");
    const req = {
        t: kInitDecoderReq,
        c: this.fileInfo.chunkSize
    }
    this.decodeWorker.postMessage(req);
};


Player.prototype.onInitDecoder = function (objData) {
    if (this.playerState === playerStateIdle) {
        return;
    }

    // this.logger.logInfo("Init decoder response " + objData.e + ".");
    if (objData.e === 0) {
        // this.logger.logInfo("Opening decoder.");
        const req = {
            t: kOpenDecoderReq
        };
        this.decodeWorker.postMessage(req);
    } else {
        this.reportPlayError(objData.e);
    }
};

Player.prototype.onOpenDecoder = function (objData) {
    if (this.playerState === playerStateIdle) {
        return;
    }

    // this.logger.logInfo("Open decoder response " + objData.e + ".");
    if (objData.e === 0) {
        this.createAudioPlayer(objData.a);
        this.logger.logInfo("Decoder ready now.");
    } else {
        this.reportPlayError(objData.e);
    }
};


Player.prototype.createAudioPlayer = function (a) {
    if (this.playerState === playerStateIdle) {
        return;
    }

    this.logger.logInfo("Audio param sampleFmt:" + a.f + " channels:" + a.c + " sampleRate:" + a.r + ".");

    var sampleFmt = a.f;
    var channels = a.c;
    var sampleRate = a.r;

    var encoding = "16bitInt";
    switch (sampleFmt) {
        case 0:
            encoding = "8bitInt";
            break;
        case 1:
            encoding = "16bitInt";
            break;
        case 2:
            encoding = "32bitInt";
            break;
        case 3:
            encoding = "32bitFloat";
            break;
        default:
            this.logger.logError("Unsupported audio sampleFmt " + sampleFmt + "!");
    }
    this.logger.logInfo("Audio encoding " + encoding + ".");

    this.audioEncoding = encoding;
    this.audioChannels = channels;
    this.audioSampleRate = sampleRate;

    this.restartAudio();
};

Player.prototype.restartAudio = function () {
    if (this.pcmPlayer) {
        this.pcmPlayer.destroy();
        this.pcmPlayer = null;
    }

    this.pcmPlayer = new PCMPlayer({
        encoding: this.audioEncoding,
        channels: this.audioChannels,
        sampleRate: this.audioSampleRate,
        flushingTime: 300
    });
};


Player.prototype.displayAudioFrame = function (frame) {
    if (this.playerState !== playerStatePlaying) {
        return false;
    }

    if (this.pcmPlayer){
        this.pcmPlayer.feed(new Uint8Array(frame.d));
    }
    return true;
};

Player.prototype.onAudioFrame = function (frame) {
    if (!this.muted)
        this.displayAudioFrame(frame)
};


Player.prototype.onVideoFrame = function (frame) {
    this.frameBuffer = frame;
};

Player.prototype.displayVideoFrame = function (frame) {
    if (this.playerState !== playerStatePlaying) {
        return false;
    }

    const data = new Uint8Array(frame.d);
    const width = frame.w;
    const height = frame.h;
    this.renderVideoFrame(data, width, height)
    return true;
};

Player.prototype.displayLoop = function () {
    // requestAnimationFrame may be 60fps, if stream fps too large,
    // we need to render more frames in one loop, otherwise display
    // fps won't catch up with source fps, leads to memory increasing,
    // set to 2 now.
    requestAnimationFrame(this.displayLoop.bind(this));
    if (this.playerState !== playerStatePlaying) {
        return;
    }

    if (this.frameBuffer) {
        this.displayVideoFrame(this.frameBuffer);
    }
};

Player.prototype.renderVideoFrame = function (data, w, h) {
    if (this.webglPlayer)
        this.webglPlayer.renderFrame(data, w, h)
};


Player.prototype.reportPlayError = function (error, status, message) {
    const e = {
        error: error || 0,
        status: status || 0,
        message: message
    };

    if (this.callback) {
        this.callback(e);
    }
};

Player.prototype.registerVisibilityEvent = function (cb) {
    let hidden = "hidden";

    // Standards:
    if (hidden in document) {
        document.addEventListener("visibilitychange", onchange);
    } else if ((hidden = "mozHidden") in document) {
        document.addEventListener("mozvisibilitychange", onchange);
    } else if ((hidden = "webkitHidden") in document) {
        document.addEventListener("webkitvisibilitychange", onchange);
    } else if ((hidden = "msHidden") in document) {
        document.addEventListener("msvisibilitychange", onchange);
    } else if ("onfocusin" in document) {
        // IE 9 and lower.
        document.onfocusin = document.onfocusout = onchange;
    } else {
        // All others.
        window.onpageshow = window.onpagehide = window.onfocus = window.onblur = onchange;
    }

    function onchange(evt) {
        const v = true;
        const h = false;
        const evtMap = {
            focus: v,
            focusin: v,
            pageshow: v,
            blur: h,
            focusout: h,
            pagehide: h
        };

        evt = evt || window.event;
        let visible = v;
        if (evt.type in evtMap) {
            visible = evtMap[evt.type];
        } else {
            visible = this[hidden] ? h : v;
        }
        cb(visible);
    }

    // set the initial state (but only if browser supports the Page Visibility API)
    if (document[hidden] !== undefined) {
        onchange({type: document[hidden] ? "blur" : "focus"});
    }
}


Player.prototype.requestStream = function (url) {
    const self = this;
    // ws，wss协议。 websocket拉取流
    this.streamSource = new WebSocket(url);
    this.streamSource.binaryType = 'arraybuffer'
    this.streamSource.onopen = function (evt) {
        self.logger.logInfo("Connection open " + url);
        // ws.send("send WebSockets!");
    };
    this.streamSource.onmessage = function (evt) {
        const objData = {
            t: kFeedDataReq,
            d: evt.data
        };
        self.decodeWorker.postMessage(objData, [objData.d])
    };
    this.streamSource.onclose = function (evt) {
        self.logger.logInfo("Connection closed.");
    };
};

Player.prototype.releaseStream = function () {
    if (this.streamSource) {
        this.streamSource.close();
        this.streamSource = null;
    }
}
