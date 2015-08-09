using ROOTNET;

namespace RDNStringDirect
{
    class Program
    {
        static void Main(string[] args)
        {
            var t = new NTStopwatch();
            var h = new NTH1F("hi", "there", 10, 0.0, 100.0);
            h.Fill(4);
            var a = h.GetXaxis();

            string newname = "fork";
            t.Start();
            for (int i = 0; i < 30000; i++)
            {
                a.SetBinLabel(3, newname);
            }
            t.Stop();
            t.Print();
        }
    }
}
