package cs6203.utils;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;

import org.apache.commons.io.IOUtils;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;

public class GOPRecordReader extends RecordReader<Text, GOP> {
	private long start;
	private long pos;
	private long end;
	private ArrayList<Integer> gopHeader;
	private int pivot = 0;
	FSDataInputStream fileIn;

	private Text key = new Text();
	private GOP value = new GOP();

	@Override
	public void close() throws IOException {
		// TODO Auto-generated method stub

	}

	@Override
	public Text getCurrentKey() throws IOException, InterruptedException {
		return key;
	}

	@Override
	public GOP getCurrentValue() throws IOException, InterruptedException {
		return value;
	}

	@Override
	public float getProgress() throws IOException, InterruptedException {
		if (start == end) {
			return 0.0f;
		} else {
			return Math.min(1.0f, (pos - start) / (float) (end - start));
		}
	}

	@Override
	public void initialize(InputSplit genericSplit, TaskAttemptContext context)
			throws IOException, InterruptedException {
		// This InputSplit is a FileInputSplit
		FileSplit split = (FileSplit) genericSplit;

		// Retrieve configuration, and Max allowed
		Configuration job = context.getConfiguration();

		// Split "S" is responsible for all records
		// starting from "start" and "end" positions
		start = split.getStart();
		end = start + split.getLength();

		// Retrieve file containing Split "S"
		final Path file = split.getPath();
		FileSystem fs = file.getFileSystem(job);
		FSDataInputStream in = fs.open(split.getPath());

		GOPFinder gopf = new GOPFinder();
		gopHeader = gopf.getGOPStartLocation(in);
		pivot = 0;
		fs.close();

		fileIn = fs.open(split.getPath());
		System.out.print("List GOP pos: ");
		for (int i = 0; i < gopHeader.size(); i++) {
			System.out.print(gopHeader.get(i));
			int s, d, length;
			if (i == 0) {
				s = 0;
				d = gopHeader.get(i) - 1;
			} else {
				s = gopHeader.get(i - 1);
				d = gopHeader.get(i) - 1;
			}
			length = d - s + 1;
			// System.out.println("Read from " + s + " to " + d +
			// ". Total length: " + length);
			// byte[] buffer = Arrays.copyOfRange(file_as_bytes, s, d);
			// br.read(buffer, 0, d - s);
			/*
			 * byte[] buffer = new byte[d-s+1]; fileIn.read(buffer, 0, length);
			 * FileOutputStream output = new FileOutputStream(new
			 * File("output-init" + i +".mpeg")); IOUtils.write(buffer, output);
			 */
		}
		System.out.println("");

		// If Split "S" starts at byte 0, first line will be processed
		// If Split "S" does not start at byte 0, first line has been already
		// processed by "S-1" and therefore needs to be silently ignored

		boolean skipFirstLine = false;
		start = 0;
		pivot = 0;
		this.pos = start;

	}

	@Override
	public boolean nextKeyValue() throws IOException, InterruptedException {
		// Current offset is the key
		key.set(Long.toString(pos));
		int newSize = 0;
		int s = 0, d = 0;
		
		if (pos >= end ) return false;

		if (pivot == 0) {
			s = 0;
			d = gopHeader.get(pivot) -1;
		} else {
			if (pivot == gopHeader.size()) {
				s = gopHeader.get(pivot - 1);
				d = (int) end;
			} else {
				s = gopHeader.get(pivot - 1);
				d = gopHeader.get(pivot) -1;
			}
		}
		byte[] buffer = new byte[d - s + 1];
//		System.out.println("Read from " + s + " to " + d + ". Total length: " + (d - s + 1));
		fileIn.read(s, buffer, 0, d - s + 1);

		// fileIn.readFully(buffer);
		/*FileOutputStream output = new FileOutputStream(new File("outputtest"
				+ key + ".mpeg"));
		IOUtils.write(buffer, output);*/
		// Read first line and store its content to "value"
		value = new GOP(buffer);
		// Line is read, new position is set
		pivot++;
		newSize = d - s + 1;
		pos += newSize;

		if (newSize == 0) {
			// We've reached end of Split
			key = null;
			value = null;
			return false;
		} else {
			// Tell Hadoop a new line has been found
			// key / value will be retrieved by
			// getCurrentKey getCurrentValue methods
			return true;
		}
	}

}
