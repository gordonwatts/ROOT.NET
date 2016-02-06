using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using ROOTNET;
using System.IO;

namespace t_TMVA
{
    static class helpers
    {
        public static NTString AsTS (this string s)
        {
            return new NTString(s);
        }
    }
    /// <summary>
    /// To make this test work, you'll need to generate wrapper classes for:
    ///   TMVA::Factory
    ///   TString
    ///   TFile
    ///   TTree
    ///   TMVA::MethodBase
    /// </summary>
    [TestClass]
    public class Basics
    {
        [TestMethod]
        public void BookMethodReturnType()
        {
            var f = NTFile.Open("junk.root", "RECREATE");
            var a = new ROOTNET.NTMVA.NFactory("job".AsTS(), f);


            // Signal and background
            var signalFileInfo = new FileInfo(@"C: \Users\gordo\Documents\Code\Root\TMVATests\signal.training.root");
            var backgroundFileInfo = new FileInfo(@"C:\Users\gordo\Documents\Code\Root\TMVATests\background.training.root");
            var signalFile = NTFile.Open(signalFileInfo.FullName, "READ");
            var backgroundFile = NTFile.Open(backgroundFileInfo.FullName, "READ");

            var signalTree = signalFile.Get("mytree") as NTTree;
            var backgroundTree = backgroundFile.Get("mytree") as NTTree;

            a.AddSignalTree(signalTree, 1.0);
            a.AddBackgroundTree(backgroundTree, 1.0);

            a.AddVariable(new NTString("nTracks"), new NTString("Number of Tracks"), new NTString(""));
            a.AddVariable(new NTString("logR"), new NTString("CalRatio"), new NTString(""));

            //ROOTNET.NTMVA.NMethodBase;

            var m = a.BookMethod("Cuts".AsTS(), "SimpleCuts".AsTS());

            Assert.IsNotNull(m);
            Assert.AreEqual("SimpleCuts", m.Name);
        }
    }
}
