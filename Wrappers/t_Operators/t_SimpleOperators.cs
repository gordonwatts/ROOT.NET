using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Operators
{
    /// <summary>
    /// Test the operator implementation. This test will only build if TLorentzVector is built
    /// as part of the monolithic stuff!
    /// </summary>
    [TestClass]
    public class t_SimpleOperators
    {
        [TestMethod]
        public void TestAdditionWithObjects1()
        {
            var a1 = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            var a2 = new ROOTNET.NTLorentzVector(0.5, 0.5, 0.5, 0.5);
            var a3 = a1 + a2;
            Assert.AreEqual(1.5, a3.X(), "X");
            Assert.AreEqual(2.5, a3.Y(), "Y");
            Assert.AreEqual(3.5, a3.Z(), "Z");
            Assert.AreEqual(4.5, a3.T(), "T");
        }

        [TestMethod]
        public void TestAdditionWithObjects2()
        {
            var a1 = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            var a2 = new ROOTNET.NTLorentzVector(0.5, 0.5, 0.5, 0.5);
            var a3 = a2 + a1;
            Assert.AreEqual(1.5, a3.X(), "X");
            Assert.AreEqual(2.5, a3.Y(), "Y");
            Assert.AreEqual(3.5, a3.Z(), "Z");
            Assert.AreEqual(4.5, a3.T(), "T");
        }

        [TestMethod]
        public void TestAdditionWithObjectInterfaces1()
        {
            var a1 = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            ROOTNET.Interface.NTLorentzVector a2 = (new ROOTNET.NTLorentzVector(0.5, 0.5, 0.5, 0.5));
            var a3 = a1 + a2;
            Assert.AreEqual(1.5, a3.X(), "X");
            Assert.AreEqual(2.5, a3.Y(), "Y");
            Assert.AreEqual(3.5, a3.Z(), "Z");
            Assert.AreEqual(4.5, a3.T(), "T");
        }

        [TestMethod]
        public void TestSubtraction1()
        {
            var a1 = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            var a2 = new ROOTNET.NTLorentzVector(0.5, 0.5, 0.5, 0.5);
            var a3 = a1 - a2;
            Assert.AreEqual(0.5, a3.X(), "X");
            Assert.AreEqual(1.5, a3.Y(), "Y");
            Assert.AreEqual(2.5, a3.Z(), "Z");
            Assert.AreEqual(3.5, a3.T(), "T");
        }

        [TestMethod]
        public void TestSubtraction2()
        {
            var a1 = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            var a2 = new ROOTNET.NTLorentzVector(0.5, 0.5, 0.5, 0.5);
            var a3 = a2 - a1;
            Assert.AreEqual(-0.5, a3.X(), "X");
            Assert.AreEqual(-1.5, a3.Y(), "Y");
            Assert.AreEqual(-2.5, a3.Z(), "Z");
            Assert.AreEqual(-3.5, a3.T(), "T");
        }

        [TestMethod]
        public void TestUnary()
        {
            var a1 = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            var a3 = -a1;
            Assert.AreEqual(-1.0, a3.X(), "X");
            Assert.AreEqual(-2.0, a3.Y(), "Y");
            Assert.AreEqual(-3.0, a3.Z(), "Z");
            Assert.AreEqual(-4.0, a3.T(), "T");
        }

        [TestMethod]
        public void TestDoubleTimes()
        {
            var a1 = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            var a3 = a1*a1;
            Assert.AreEqual(16.0 - 1.0 - 4.0 - 9.0, a3, "magnitude");
        }

        [TestMethod]
        public void TestScaling()
        {
            var a1 = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            var a3 = 2.0 * a1;
            Assert.AreEqual(2.0, a3.X(), "X");
            Assert.AreEqual(4.0, a3.Y(), "Y");
            Assert.AreEqual(6.0, a3.Z(), "Z");
            Assert.AreEqual(8.0, a3.T(), "T");
        }

#if false
        [TestMethod]
        public void TestAdditionWithObjectInterfaces2()
        {
            ROOTNET.Interface.NTLorentzVector a1 = new ROOTNET.NTLorentzVector(1.0, 2.0, 3.0, 4.0);
            ROOTNET.NTLorentzVector a2 = new ROOTNET.NTLorentzVector(0.5, 0.5, 0.5, 0.5);
            var a3 = a1 + a2;
            Assert.AreEqual(1.5, a3.X(), "X");
            Assert.AreEqual(2.5, a3.Y(), "Y");
            Assert.AreEqual(3.5, a3.Z(), "Z");
            Assert.AreEqual(4.5, a3.T(), "T");
        }
#endif
    }
}
