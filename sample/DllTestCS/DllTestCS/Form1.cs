using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.Text;
using System.Windows.Forms;
using jp.nyatla.cs.NyWMCapture;
using System.Threading;
namespace DllTestCS
{

    public partial class Form1 : Form, INySampleCB,IDisposable
    {
        
        //終了タイミングの同期を取るための仕掛け
        bool _dispose_request = false;
        bool _dispose_ready = false;
        //
        NyWMCapture  capture;
        public INyWMCapture capif;
        PictureBox   pictureBox1;
        public Form1()
        {
            InitializeComponent();
            this.pictureBox1 = new PictureBox();
            this.pictureBox1.Size = new Size(240, 320);
            this.Controls.Add(this.pictureBox1);
            this.capture = new NyWMCapture();
            this.capif = (INyWMCapture)capture;
            int hr;
            hr = this.capif.SetSize(240, 320);
            hr = this.capif.SetCallBack(this);

            hr = this.capif.Initialize(
                NyWMCapture.DeviceId_WM5,
                NyWMCapture.MediaSubType_RGB565,
                NyWMCapture.PinCategory_PREVIEW);
            hr = this.capif.Start();
            return;
        }
        Bitmap bmp=new Bitmap(240,320);
        private delegate void myDelegate();
        public void dg()
        {
            this.pictureBox1.Image = bmp;
            return;
        }
        public int OnSample(INySample i_sample)
        {
            BitmapData bd = this.bmp.LockBits(
                new Rectangle(0, 0,240,320),
                ImageLockMode.WriteOnly,
                PixelFormat.Format16bppRgb565);
            //ビットマップデータのコピー
            i_sample.CopyToBuffer(bd.Scan0, 0, i_sample.GetDataSize());
            this.bmp.UnlockBits(bd);
            //メインスレッドに同期して画面更新
            if (this._dispose_request)
            {
                this._dispose_ready = true;
            }
            else
            {
                myDelegate d = new myDelegate(dg);
                this.Invoke(d);
            }
            return 0;
        }        
        public new void Dispose()
        {
            //delegate呼び出しを抑制する。
            this._dispose_request = true;
            while (!this._dispose_ready)
            {
                //OnSampleがdelegateでブロックしていれば解消する。
                Application.DoEvents();
            }
            this.capif.Stop();
            this.capif.Finalize();
            System.Runtime.InteropServices.Marshal.ReleaseComObject(this.capif);
            this.capif = null;
            base.Dispose();
            return;
        }
        private void menuItem2_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void Form1_KeyDown(object sender, KeyEventArgs e)
        {
            if ((e.KeyCode == System.Windows.Forms.Keys.Up))
            {
                // Up
            }
            if ((e.KeyCode == System.Windows.Forms.Keys.Down))
            {
                // Down
            }
            if ((e.KeyCode == System.Windows.Forms.Keys.Left))
            {
                // Left
            }
            if ((e.KeyCode == System.Windows.Forms.Keys.Right))
            {
                // Right
            }
            if ((e.KeyCode == System.Windows.Forms.Keys.Enter))
            {
                // Enter
            }

        }
    }
        
}