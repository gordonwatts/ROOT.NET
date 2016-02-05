using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using ROOTNET;

namespace t_TString
{
    [TestClass]
    public class ConversionTests
    {
        // When bug #7 is done, we can uncomment this.
        [TestMethod]
        public void SimpleTStringCTor()
        {
            var t = new NTString("hi");
            Assert.AreEqual("hi", t.Data());
        }
#if false

        class ImplicitTestForTheObject
        {
            public NTString _s;

            public ImplicitTestForTheObject(NTString obj)
            {
                _s = obj;
            }
        }

        [TestMethod]
        public void SimpleTStringImplicitConversion()
        {
            var o = new ImplicitTestForTheObject("hi");
            Assert.AreEqual("hi", o._s.Data());
        }

        class ImplicitTestConvertToString
        {
            public string _s;
            public ImplicitTestConvertToString(string s)
            {
                _s = s;
            }
        }

        [TestMethod]
        public void SimpleExplicitStringConversion()
        {
            var t = new NTString("hi");
            var o = new ImplicitTestConvertToString((string)t);
            Assert.AreEqual("hi", o._s);

        }

        [TestMethod]
        public void SimpleImplicitStringConversion()
        {
            var t = new NTString("hi");
            var o = new ImplicitTestConvertToString(t);
            Assert.AreEqual("hi", o._s);

        }

        class ImplicitTestForTheObjectInterface
        {
            public ROOTNET.Interface.NTString _s;

            public ImplicitTestForTheObjectInterface(ROOTNET.Interface.NTString obj)
            {
                _s = obj;
            }
        }

        [TestMethod]
        public void SimpleTStringImplicitConversionInterface()
        {
            var o = new ImplicitTestForTheObjectInterface("hi".AsTS());
            Assert.AreEqual("hi", o._s.Data());
        }
#endif
    }
}
