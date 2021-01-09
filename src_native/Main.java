import java.nio.ByteBuffer;

class Main {
  public static void main(String[] args) {
    shmem mem = new shmem();
    long handle = mem.get_handle();

    while (true) {
      while (!mem.get_next_frame(handle)) {
        try {
          Thread.sleep(50);
        } catch(InterruptedException ex) {
          Thread.currentThread().interrupt();
        }   
      }

      ByteBuffer[] content_buffer = mem.get_content_buffer(handle);
      ByteBuffer[] foreground_color_buffer = mem.get_foreground_color_buffer(handle);
      ByteBuffer[] background_color_buffer = mem.get_background_color_buffer(handle);

      System.out.print(".\r");
      System.out.print((char)content_buffer[0].get(0));
      System.out.print((char)content_buffer[0].get(1));
      System.out.print((char)content_buffer[0].get(2));
      System.out.print((char)content_buffer[0].get(3));
      System.out.print(" ");
      System.out.print(foreground_color_buffer[0].asIntBuffer().get(0));
      System.out.print(" ");
      System.out.print(background_color_buffer[0].asIntBuffer().get(0));
      System.out.print("                             ");
    }
  }
}
