# dym2149-ym-streamer
Simple Windows C++ application to feed [Dual YM2149 synth](https://github.com/bderleta/dym2149-board) with one or two YM files via USB UART.
Requires Visual C++, has been built with latest (Community 2022) version.

It accepts **decompressed** YM files following the [YM file format specification](http://leonard.oxg.free.fr/ymformat.html).
Invoke it via `YmStreamer.exe <file1> <file2> \\.\COMx`. If you want to play same tune on two PSGs in parallel, just specify same filename twice.
