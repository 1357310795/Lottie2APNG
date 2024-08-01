# ![lottie.png](https://s2.loli.net/2024/08/01/utCJisV4HPprMay.png) Lottie to APNG Converter

Open your lottie file, adjust settings and export in one minute.

![](https://s2.loli.net/2024/08/01/bjEdXJ2u9vsCqS3.png)

## Samples
|||||
|---|---|---|---|
|![](./images/311.png)|![](./images/314.png)|![](./images/317.png)|![](./images/319.png)|
|![](./images/320.png)|![](./images/324.png)|![](./images/337.png)|![](./images/341.png)|

## Known issues
Exporting at the second time is broken now...

## Linux support
Of course the converter is cross-platform. The GUI is using WPF and is only available on Windows. If you want to use the converter in linux, currently you have to write your own code (extract the function `Run` in `MainWindows.xaml.cs`)

## Future plan
- Implement the converter fully in C#
- Speed up by multithreading
