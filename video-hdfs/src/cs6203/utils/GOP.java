package cs6203.utils;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Arrays;

import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableUtils;

public class GOP  implements Writable {
	byte[] value;
	long size;

	public GOP(byte[] buffer) {
		size = buffer.length;
		value = Arrays.copyOf(buffer,(int) size);
	}
	
	public byte[] getValue() {
		return value;
	}
	
	public long getSize() {
		return size;
	}	
	
	public GOP() {
		
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		in.readFully(value);
		
	}

	@Override
	public void write(DataOutput out) throws IOException {
		out.write(value);
		
	}	

}