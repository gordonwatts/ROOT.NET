using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using ROOTNET;
using System.IO;

namespace t_Tree
{
    /// <summary>
    /// Summary description for UnitTest1
    /// </summary>
    [TestClass]
    public class t_BasicTreeOperations
    {
        [TestMethod]
        [DeploymentItem("btag-slim.root")]
        public void TestCountEntriesForEach()
        {
            // Open and get the tree
            var tree = Utils.OpenAndGet("btag-slim.root", "vtuple");
            int count = 0;
            foreach (var evt in tree)
            {
                count++;
            }
            Assert.AreEqual(100, count, "# of events in the tree");
        }

        [TestMethod]
        [DeploymentItem("btag-slim.root")]
        public void TestCountEntriesLINQ()
        {
            // Open and get the tree
            var tree = Utils.OpenAndGet("btag-slim.root", "vtuple");
            int count = tree.Count();
            Assert.AreEqual(100, count, "# of events in the tree");
        }

        [TestMethod]
        [DeploymentItem("btag-slim.root")]
        public void TestCountEntriesLINQTwice()
        {
            // Open and get the tree
            var tree = Utils.OpenAndGet("btag-slim.root", "vtuple");
            int count = tree.Count();
            Assert.AreEqual(100, count, "# of events in the tree");
            count = tree.Count();
            Assert.AreEqual(100, count, "# of events in the tree");
        }
    }
}
