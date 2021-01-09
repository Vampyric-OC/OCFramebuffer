import java.nio.ByteBuffer;

public class shmem {
  static {
    try {
      System.loadLibrary("shmem");
    	//System.load(System.getProperty("user.dir") + "/shmem.dll");
    } catch (Exception e) {
      System.err.println("Library failed to load.");
      System.exit(1);
    }
  }

  public native long get_handle();
  public native boolean get_next_frame(long handle);
  public native ByteBuffer[] get_content_buffer(long handle);
  public native ByteBuffer[] get_foreground_color_buffer(long handle);
  public native ByteBuffer[] get_background_color_buffer(long handle);
}
