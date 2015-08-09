using Microsoft.CSharp.RuntimeBinder;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Dynamic
{
    /// <summary>
    /// Tests to make sure method invokation works correctly, as well as argument
    /// passing and returns.
    /// </summary>
    [TestClass]
    public class t_callingMethods
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
        public void TestIntegerReturns()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F();
            Assert.AreEqual(1, c.GetAxisColor(), "Axis color reset");
        }

        [TestMethod]
        public void TestVoidReturns()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F();
            c.SetAxisColor(2);
            Assert.AreEqual(2, c.GetAxisColor(), "Axis color reset");
        }

        [TestMethod]
        [ExpectedException(typeof(RuntimeBinderException))]
        public void TestBadMethodName()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F();
            c.SetAxisColors(2);
        }

        [TestMethod]
        public void StringArgument()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F();
            c.SetOption("dude");
            Assert.AreEqual("dude", c.GetOption(), "object string name");
        }

        [TestMethod]
        public void StringArgumentReturn()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TAxis();
            c.Set(10, 1.0, 11.0);
            c.SetBinLabel(1, "hi");
            Assert.AreEqual("hi", c.GetBinLabel(1), "Bin name");
        }

        [TestMethod]
        public void TObjectReturnType()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F();
            dynamic a = c.GetXaxis();
            Assert.IsNotNull(a);
            a.Set(10, 1.0, 11.0); // Make sure it really is a TAxis
            a.SetBinLabel(1, "hi");
            Assert.AreEqual("hi", a.GetBinLabel(1), "Bin name");
        }

        [TestMethod]
        public void IntAsDoubleArg()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F();
            c.SetBins(10, 0.0, 10.0);
            c.Fill(4);
            Assert.AreEqual(1.0, c.GetBinContent(5), "Bin #4");
        }

        [TestMethod]
        public void TObjectAsArg()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F();
            c.SetBins(1, 0.0, 10.0);
            c.Fill(4.0);

            dynamic c2 = c.Clone();
            c.Divide(c2);

            Assert.AreEqual(1.0, c.GetBinContent(1), "Divided bin value");
        }
    }
}
