using System.Collections;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using Teru.Code.Models;

namespace Lottie2APNG.LibWrapper
{
    public class ApngManager
    {
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public struct ApiResult
        {
            public byte success;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string message;
        }

        [DllImport("Lottie2APNG.APNGASM.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr CreateManager();        

        [DllImport("Lottie2APNG.APNGASM.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ApiResult DestroyManager(IntPtr inst);

        [DllImport("Lottie2APNG.APNGASM.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ApiResult SetSize(IntPtr inst, int width, int height);

        [DllImport("Lottie2APNG.APNGASM.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ApiResult SetLoops(IntPtr inst, int loops);

        [DllImport("Lottie2APNG.APNGASM.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ApiResult SetSkipFirst(IntPtr inst, int first);

        [DllImport("Lottie2APNG.APNGASM.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ApiResult SetFps(IntPtr inst, int num, int den);
        
        [DllImport("Lottie2APNG.APNGASM.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ApiResult SetOptimizationOptions(IntPtr inst, bool keep_palette, bool keep_coltype);
        
        [DllImport("Lottie2APNG.APNGASM.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ApiResult SetCompressionOptions(IntPtr inst, int deflate_method, int iter);

        [DllImport("Lottie2APNG.APNGASM.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ApiResult AddFrame(IntPtr inst, IntPtr data, int count);

        [DllImport("Lottie2APNG.APNGASM.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ApiResult AddFrameFromFile(IntPtr inst, string filename);
        
        [DllImport("Lottie2APNG.APNGASM.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ApiResult Optimize(IntPtr inst);
        
        [DllImport("Lottie2APNG.APNGASM.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern ApiResult Save(IntPtr inst, string filename, ref int cur, ref int total);


        private IntPtr ptr;
        public ApngManager()
        {
            ptr = CreateManager();
        }

        ~ApngManager()
        {
            DestroyManager(ptr);
        }

        public CommonResult SetSize(int width, int height)
        {
            var res = SetSize(ptr, width, height);
            return new CommonResult(res.success == 1, res.message);
        }

        public CommonResult SetLoops(int loops)
        {
            var res = SetLoops(ptr, loops);
            return new CommonResult(res.success == 1, res.message);
        }

        public CommonResult SetSkipFirst(int first)
        {
            var res = SetSkipFirst(ptr, first);
            return new CommonResult(res.success == 1, res.message);
        }

        public CommonResult SetFps(int num, int den)
        {
            var res = SetFps(ptr, num, den);
            return new CommonResult(res.success == 1, res.message);
        }

        public CommonResult SetOptimizationOptions(bool keep_palette, bool keep_coltype)
        {
            var res = SetOptimizationOptions(ptr, keep_palette, keep_coltype);
            return new CommonResult(res.success == 1, res.message);
        }

        public CommonResult SetCompressionOptions(int deflate_method, int iter)
        {
            var res = SetCompressionOptions(ptr, deflate_method, iter);
            return new CommonResult(res.success == 1, res.message);
        }

        public CommonResult AddFrame(MemoryStream ms)
        {
            byte[] buffer = ms.GetBuffer();
            int length = (int)ms.Length;
            //IntPtr pointer = Marshal.AllocHGlobal(length);
            //Marshal.Copy(buffer, 0, pointer, length);

            //// 调用C++函数
            //var res = AddFrame(ptr, pointer, length);

            //// 释放分配的内存
            //Marshal.FreeHGlobal(pointer);

            GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
            IntPtr bytePtr = handle.AddrOfPinnedObject();
            // 调用C++函数
            var res = AddFrame(ptr, bytePtr, length);

            handle.Free(); // 操作完成后，不要忘记释放GCHandle

            return new CommonResult(res.success == 1, res.message);
        }

        public CommonResult Optimize()
        {
            var res = Optimize(ptr);
            return new CommonResult(res.success == 1, res.message);
        }

        public CommonResult Save(string filename, ref int cur, ref int total)
        {
            var res = Save(ptr, filename, ref cur, ref total);
            return new CommonResult(res.success == 1, res.message);
        }
    }
}
