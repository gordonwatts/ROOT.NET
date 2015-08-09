using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.CSharp.RuntimeBinder;

namespace t_Dynamic
{
    /// <summary>
    /// Test properties on an object
    /// </summary>
    [TestClass]
    public class t_Properties
    {
        // Force a reference to the monolithic wrapper guys - so that we can
        // make sure that the wrappers get loaded into memory.
#pragma warning disable 0414
        static ROOTNET.NTObject _obj = null;
#pragma warning restore 0414

        [TestInitialize]
        public void TestInit()
        {
            ROOTNET.Utility.ROOTCreator.ResetROOTCreator();
        }

        [TestMethod]
        public void TestGetStrippedPropertyName()
        {
            var h1 = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F("hi", "there", 100, 0.0, 10.0);
            Assert.AreEqual(100, h1.GetNbinsX, "x bins");
        }

        [TestMethod]
        [ExpectedException(typeof(RuntimeBinderException))]
        public void TestGetBogusPropertyName()
        {
            var tlz = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            var dtlz = (dynamic)tlz;
            var r = dtlz.XYBogus;
        }

        [TestMethod]
        [ExpectedException(typeof(RuntimeBinderException))]
        public void TestSetBogusPropertyName()
        {
            var tlz = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            var dtlz = (dynamic)tlz;
            dtlz.XYBogus = 10.0;
        }

        [TestMethod]
        public void TestGetPropertyName()
        {
            var h1 = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F ("hi", "there", 100, 0.0, 10.0);
            Assert.AreEqual(100, h1.NbinsX, "# of bins");
            h1.Fill(5.0);
            Assert.AreEqual(1.0, h1.Maximum, "Maximum");
        }

        [TestMethod]
        public void TestSetPropertyName()
        {
            var h1 = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F("hi", "there", 100, 0.0, 10.0);
            h1.Maximum = 3.0;
            Assert.AreEqual(3.0, h1.Maximum, "Maximum after being set");
        }
    }
}
