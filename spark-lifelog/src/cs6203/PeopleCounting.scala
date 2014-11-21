package cs6203

import org.apache.spark.SparkContext
import org.apache.spark.SparkContext._
import java.io.File
import java.io.FileInputStream
import java.nio.channels.FileChannel.MapMode._
import java.io.RandomAccessFile
import java.nio.channels.FileChannel
import scala.sys.process._
import org.apache.spark.SparkConf
import java.nio.ByteBuffer
import org.apache.hadoop.mapred.TextInputFormat
import org.apache.hadoop.io.LongWritable
import org.apache.hadoop.io.Text
import cs6203.utils.GOPInputFormat
import cs6203.utils.GOP


object PeopleCounting {
  
  val gopSize = 64000000 //64MB
  
  def parseMatlabResult(ret : String):(Int, Array[String]) = {
    val splitted = ret.split("\n");
   // for ( x <- splitted ) {
    def findPivot : Int = {
      for (i <- 0 until splitted.length by 1) {
        if (splitted(i).startsWith("LD_LIBRARY_PATH")) {
          return i
        }
      }
      0
    }
    
    val index = findPivot
    println("index " + index)
    val mlOutput = splitted.slice(index+1, splitted.size)
    (mlOutput(0).toInt,mlOutput.slice(1, mlOutput.size))    
  }
  
  // => (key, (Int, BoundingBoxes list))
  //def peopleDetection (gop: ByteBuffer, cmd : String) : Int = {
  def peopleDetection (key: Text, gop: GOP) : (Text, Int) = {

    val file = new File("/home/vutrang/somefile.dat")
    file.delete()
    val fc = new RandomAccessFile(file, "rw").getChannel()
    val mbb = fc.map(FileChannel.MapMode.READ_WRITE, 0, gopSize)
    mbb.put(gop.getValue())
    
    val cmd = "/home/vutrang/today/CS6203-git/matlabcode/run_demo_for_spark.sh /usr/local/MATLAB/R2012a /home/vutrang/today/CS6203-git/Test/pos/person_350.png"
    //val abc = scala.sys.process.Process( cmd +  " /home/vutrang/somefile.dat")
    
    
    //val cmd = "uname -a"
    val output = cmd.!!
    val parsed = parseMatlabResult(output);
    println("number bb: " + parsed._1)
    for (bb <- parsed._2) {
    	println(bb)
    }
    mbb.put(0, 0);
    fc.close()
    (key,parsed._1)
  }
  
  def main(args: Array[String]) {

/*    if (args.length < 4) {
      System.err.println("Usage: PeopleCounting <master> <matlab_path> <input> <output>")
      System.exit(1)
    }*/
    val matlab = "/usr/local/MATLAB/R2012a" //args(1)
    val input = "MCT_part1.mpeg"//args(2)
    val output = "output" //args(3)
    
    //Init Spark application
    val conf = new SparkConf().setMaster("local")//setMaster(args(0))
      .setAppName("PeopleCounting")
      .setJars(SparkContext.jarOfClass(this.getClass))
    //conf.set("spark.scheduler.mode", "FAIR")
    val sc = new SparkContext(conf)
    
    //val gops = sc.hadoopRDD(conf, GOPInputFormat.getClass, String, GOP, 0)
    
    val gops = sc.newAPIHadoopFile(input, classOf[GOPInputFormat], classOf[Text], classOf[GOP])
    val ml = gops.map(pair => peopleDetection(pair._1, pair._2))
    ml.count()
    
  }
}