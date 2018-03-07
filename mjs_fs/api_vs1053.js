let VS1053 = {






    _create: ffi('void *mgos_vs1053_create()'),
    _begin: ffi('int mgos_vs1053_begin(void*)'),
    _close: ffi('void mgos_vs1053_close(void*)'),
    _playFile: ffi('void mgos_vs1053_playFile(void*, char*)'),

    _proto: {

        // ## **`VS1053.begin()`**
        begin: function () {
            VS1053._begin(this._vs1053);
        },

        close: function () {
            VS1053._close(this._vs1053);
        },

        playFile: function (filename){
            VS1053._playFile(this._vs1053, filename);
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
