package cs6203.utils;

import java.io.IOException;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;

public class GOPInputFormat extends FileInputFormat<Text, GOP> {

	@Override
	public RecordReader<Text, GOP> createRecordReader(InputSplit split,
			TaskAttemptContext context) throws IOException, InterruptedException {
		 GOPRecordReader reader = new GOPRecordReader();
		 reader.initialize(split, context);
		return reader;
	}

}
