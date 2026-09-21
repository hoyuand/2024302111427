using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

public static class TerminalWindowCapture
{
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }

    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);

    [DllImport("user32.dll")]
    public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBlt, uint flags);

    [DllImport("user32.dll")]
    public static extern bool ShowWindow(IntPtr hWnd, int command);

    public static void Save(IntPtr handle, string path)
    {
        RECT rect;
        if (!GetWindowRect(handle, out rect)) throw new InvalidOperationException("GetWindowRect failed");
        int width = rect.Right - rect.Left;
        int height = rect.Bottom - rect.Top;
        using (Bitmap bitmap = new Bitmap(width, height))
        using (Graphics graphics = Graphics.FromImage(bitmap))
        {
            IntPtr hdc = graphics.GetHdc();
            try
            {
                if (!PrintWindow(handle, hdc, 2)) throw new InvalidOperationException("PrintWindow failed");
            }
            finally
            {
                graphics.ReleaseHdc(hdc);
            }
            bitmap.Save(path, ImageFormat.Png);
        }
    }
}
