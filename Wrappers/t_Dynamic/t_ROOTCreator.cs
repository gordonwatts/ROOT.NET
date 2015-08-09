using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Dynamic
{
    /// <summary>
    /// Make sure various things with ctor and dtor are working. For a real test
    /// the monilithic wrapper guy should be pretty minimal.
    /// </summary>
    [TestClass]
    public class t_ROOTCreator
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
        public void CreateTH1FNoArgs()
        {
            dynamic c = new ROOTNET.Utility.ROOTCreator();
            var h = c.TH1F();
            Assert.AreEqual(1, h.GetNbinsX(), "# of bins"); // 1 is the default
        }

        [TestMethod]
        public void CreateTH1FNoArgsByName()
        {
            dynamic h = ROOTNET.Utility.ROOTCreator.CreateByName("TH1F", new object[0]);
            Assert.AreEqual(1, h.GetNbinsX(), "# of bins");
        }

        [TestMethod]
        [ExpectedException(typeof(ROOTNET.Utility.ROOTDynamicException))]
        public void TestCreatorCTorBadClass()
        {
            dynamic c = new ROOTNET.Utility.ROOTCreator();
            dynamic h = c.TH1FF();
        }

        [TestMethod]
        [ExpectedException(typeof(ROOTNET.Utility.ROOTDynamicException))]
        public void NoNonTObjectClasses()
        {
            dynamic c = new ROOTNET.Utility.ROOTCreator();
            var h = c.TArray();
            Assert.IsNull(h, "no object should have been created");
        }

        [TestMethod]
        public void CreateTH1FByProperty()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F();
            Assert.AreEqual(1, c.GetNbinsX(), "# of bins"); // 1 is the default
        }

        [TestMethod]
        [ExpectedException(typeof(ROOTNET.Utility.ROOTDynamicException))]
        public void NoSuchCtor()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TObjString("dude", 57);
        }

        [TestMethod]
        public void SingleStringArgCtor()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TObjString("dude");
            Assert.AreEqual("dude", c.GetName(), "Name stored");
        }

        [TestMethod]
        public void MulitipleArgCtor()
        {
            dynamic c = ((dynamic)ROOTNET.Utility.ROOTCreator.ROOT).TH1F("hi", "there", 100, 0.0, 100.0);
            Assert.AreEqual("hi", c.GetName(), "Name stored");
            Assert.AreEqual("there", c.GetTitle(), "Title stored");
            Assert.AreEqual(100, c.GetNbinsX(), "# of bins");
            Assert.AreEqual(0.0, c.GetBinLowEdge(1), "Low bin boundary");
            Assert.AreEqual(100.0, c.GetBinLowEdge(101), "High bin boundary");
        }

        [TestMethod]
        public void TestTParameterTemplate()
        {
            dynamic c = (dynamic)ROOTNET.Utility.ROOTCreator.CreateByName("TParameter<int>", new object[] { "Dude", (int)5 });
            Assert.AreEqual(5, c.GetVal(), "Value of parameter");
            Assert.AreEqual("Dude", c.GetName(), "Name of the object");
        }

        [TestMethod]
        public void TestVariableParameters()
        {
            dynamic c = (dynamic)ROOTNET.Utility.ROOTCreator.CreateByName("TParameter<int>", "Dude", (int)5);
            Assert.AreEqual(5, c.GetVal(), "Value of parameter");
            Assert.AreEqual("Dude", c.GetName(), "Name of the object");
        }
    }
}
