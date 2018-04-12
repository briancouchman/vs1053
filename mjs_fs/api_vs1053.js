let VS1053 = {
    _create: ffi('void *mgos_vs1053_create()'),
    _begin: ffi('int mgos_vs1053_begin(void*)'),
    _close: ffi('void mgos_vs1053_close(void*)'),
    _playFile: ffi('void mgos_vs1053_playFile(void*, char*, void(*)(userdata, char *), userdata)'),
    _closeFile: ffi('void mgos_vs1053_closeFile(void*)'),
    _pause: ffi('void mgos_vs1053_pausePlaying(void*)'),
    _stop: ffi('void mgos_vs1053_stopPlaying(void*)'),
    _isPlaying: ffi('bool mgos_vs1053_playing(void*)'),
    _isPaused: ffi('bool mgos_vs1053_paused(void*)'),
    _isStopped: ffi('bool mgos_vs1053_stopped(void*)'),

    _proto: {

        // ## **`VS1053.begin()`**
        begin: function () {
            VS1053._begin(this._vs1053);
        },

        close: function () {
            VS1053._close(this._vs1053);
        },

        playFile: function (filename, callback, userdata){
            VS1053._playFile(this._vs1053, filename, callback, userdata);
        },

        pause: function(){
            VS1053._pause(this._vs1053);
        },

        stop: function(){
            VS1053._stop(this._vs1053);
        },

        isPlaying: function(){
            return VS1053._isPlaying(this._vs1053);
        },

        isStopped: function(){
            return VS1053._isStopped(this._vs1053);
        },

        isPaused: function(){
            return VS1053._isPaused(this._vs1053);
        }






        // read: function (filename, bufSize) {
        //     return SD._read(this.sd, filename, bufSize);
        // },
        //
        // write: function (filename, bufSize) {
        //     return SD._write(this.sd, filename, bufSize);
        // },

    },

    // ## **`VS1053.create(  )`**
    // Create an VS1053 instance.
    // Return value: an object representing the VS1053 with the methods defined in _proto: {...} (above).
    create: function () {
        let obj = null;

        obj = Object.create(VS1053._proto);
        obj._vs1053 = VS1053._create();

        return obj;
    }

};
