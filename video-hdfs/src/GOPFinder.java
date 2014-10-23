import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;

import org.apache.commons.io.IOUtils;


public class GOPFinder {
	public ArrayList<Integer> getGOPStartLocation(FileInputStream in) throws IOException {
        ArrayList<Integer> GOPLocations = new ArrayList<Integer>();

		byte file_as_bytes[] = IOUtils.toByteArray(in);
        Byte GOPflag0 = (byte)0x00;
        Byte GOPflag1 = (byte)0x00;
        Byte GOPflag2 = (byte)0x01;
        Byte GOPflag3 = (byte)0xB8;
        
        for (int i = 0; i < file_as_bytes.length - 3; i+=4){
        	Byte b0 = file_as_bytes[i];
        	Byte b1 = file_as_bytes[i+1];
        	Byte b2 = file_as_bytes[i+2];
        	Byte b3 = file_as_bytes[i+3];
        	
        	if(b0.compareTo(GOPflag0) == 0 && b1.compareTo(GOPflag1) == 0 && 
        	   b2.compareTo(GOPflag2) == 0 && b3.compareTo(GOPflag3) == 0) {
        		GOPLocations.add(i);
        		
        		/*comment out if no need */
        		System.out.println("find GOP start flag at: " + i);
        	}
        		
        }            
		
		return GOPLocations;		
	}
	
	
    public static void main(String[] args) throws IOException {
        FileInputStream in = null;
        GOPFinder finder = new GOPFinder();
        try {
        	in = new FileInputStream("/Users/shixudongleo/Downloads/test_video/MCT_part2.mpeg");
        	ArrayList<Integer> locations = finder.getGOPStartLocation(in);
                        
            System.out.println("total = " + locations.size());

        } finally {
            if (in != null) {
                in.close();
            }
        }
    }
}

