using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;

namespace temperatureTest
{
    public partial class Form1 : Form
    {
        int g_vaild_count=0;
        public Form1()
        {
            InitializeComponent();
        }

        private void btn_open_Click(object sender, EventArgs e)
        {
            //open port
            if (btn_open.Text == "open")
            {
                try
                {
                    serialPort1.BaudRate = Int32.Parse(box_baudrate.Text);
                    serialPort1.PortName = box_com.Text;
                    serialPort1.ReadTimeout = 10;
                    serialPort1.WriteTimeout = 10;
                    serialPort1.Open();
                    timer1.Enabled = true;
                    serialPort1.DiscardInBuffer();
                }
                catch (Exception re)
                {
                    MessageBox.Show("Port Open Fail:" + re.Message);
                    return;
                }
                btn_open.Text = "close";
            }
            else//close port
            {
                try
                {    
                    serialPort1.Close();
                    timer1.Enabled = false;
                }
                catch (Exception re)
                {
                    MessageBox.Show("Port Close Fail:" + re.Message);
                    return;
                }
                btn_open.Text = "open";
            }
        }

        //200ms
        private void timer1_Tick(object sender, EventArgs e)
        {
            string rec_buf = "";

            try
            {
                rec_buf = serialPort1.ReadLine();
                //       H    T     P    C
                //value=0.00,0.00,0.00,0.00
                if (rec_buf != "")
                {
                    g_vaild_count = 0;
                    int index = rec_buf.IndexOf("value=");
                    rec_buf = rec_buf.Substring(index + 6);
                    string[] values = rec_buf.Split(',');
                    try
                    {
                        txt_hum.Text = (Convert.ToDouble(values[0])).ToString();
                        txt_temp.Text = (Convert.ToDouble(values[1])).ToString();
                        txt_pm.Text = (Convert.ToDouble(values[2])).ToString();
                        txt_con.Text = (Convert.ToDouble(values[3])).ToString();
                    }
                    catch { }
                }
                }
                catch(System.TimeoutException re)
                {
                    g_vaild_count++;
                    if (g_vaild_count == 100)//200*100 = 20000 = 20s
                    {
                        try
                        {
                            serialPort1.Close();
                        }
                        catch { }
                        timer1.Enabled = false;
                        btn_open.Text = "open";
                        MessageBox.Show("Communication Fail:No Data Receive");
                    }
                }
                catch (Exception re)
                {
                    try
                    {
                        serialPort1.Close();
                    }
                    catch { }
                    timer1.Enabled = false;
                    btn_open.Text = "open";
                    MessageBox.Show("Communication Fail:" + re.Message);
                }
        }

        /// <summary>
        /// 延时函数，同时界面可以响应
        /// </summary>
        /// <param name="time">需要延时的时间 ms</param>
        void ThradDelayms(int time)
        {
            int count = time / 10;
            for (int i = 0; i < count; i++)
            {
                Thread.Sleep(10);
                Application.DoEvents();
            }
            time = time - count * 10;
            Thread.Sleep(time);
        }

    }
}
