# web-server-vst

This is a service that hosts audio plugins for DSP processing.

## This project is in Framework Juce 7 and Socket IO in C++

``
version 1.0.0
``

### For Building Follow the List:

1. Download the latest version of the Juce framework
2. Open Projucer and go to openFile and choose the web-server-vst.jucer file
3. Open the project with the build button in Visual Studio or Xcode
4. Build

### Library dependencies:

```
vstsdk2.4
```

```
Framework Juce v7.0.5
```

### VST2 Host - DLL

This service hosts different types of plugins and processes each initiated request in parallel.

