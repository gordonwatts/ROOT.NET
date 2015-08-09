using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Tree
{
    /// <summary>
    /// Test out tree's with vectors in them.
    /// </summary>
    [TestClass]
    public class t_vectorTreeVariables
    {
        [TestMethod]
        [DeploymentItem("vectorintonly_5.root")]
        public void TestVectorMethodSize()
        {
            var t = Utils.OpenAndGet("vectorintonly_5.root", "dude");
            Assert.AreEqual(5, t.Count(), "Count");
            Assert.AreEqual(5, t.Cast<dynamic>().Where(evt => evt.myvectorofint.size() == 10).Count(), "# of entries with 10 items in the vector");
        }

        [TestMethod]
        [DeploymentItem("vectorintonly_5.root")]
        public void TestVectorIndexer()
        {
            var t = Utils.OpenAndGet("vectorintonly_5.root", "dude");
            Assert.AreEqual(5, t.Count(), "Count");
            Assert.AreEqual(5, t.Cast<dynamic>().Where(evt => evt.myvectorofint[2] == 2).Count(), "# of entries with 10 items in the vector");
        }

        [TestMethod]
        [DeploymentItem("vectorintonly_5.root")]
        public void TestVectorIEnumerable()
        {
            var t = Utils.OpenAndGet("vectorintonly_5.root", "dude");
            Assert.AreEqual(5, t.Count(), "Count");
            foreach (dynamic evt in t)
            {
                int count = 0;
                foreach (var i in evt.myvectorofint)
                {
                    count++;
                }
                Assert.AreEqual(10, count, "counting everythign in here");
            }
        }

        [TestMethod]
        [DeploymentItem("vectorstring.root")]
        public void TestVectorStringsIterator()
        {
            var t = Utils.OpenAndGet("vectorstring.root", "vectorsofstrings");
            int cnt = t.Cast<dynamic>().SelectMany(evt => evt.strings as IEnumerable<Object>).Where(s => s as string == "3").Count();
            Assert.AreEqual(2, cnt, "# of times 3 appears");
        }

        [TestMethod]
        [DeploymentItem("vectorstring.root")]
        public void TestVectorStringsIndex()
        {
            var t = Utils.OpenAndGet("vectorstring.root", "vectorsofstrings");
            int cnt = t.Cast<dynamic>().Where(evt => evt.strings[0] == "3").Count();
            Assert.AreEqual(1, cnt, "# of times 3 appears in first slot");
        }
    }
}
