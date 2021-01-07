import java.nio.ByteBuffer;

class Main {
  public static void main(String[] args) {
    shmem mem = new shmem();
    long handle = mem.get_handle();

    ByteBuffer buffer = mem.get_next_frame(handle);
    
    while (buffer == null) {
      try {
        System.out.println("Sleep");
        Thread.sleep(50);
      } catch(InterruptedException ex) {
        Thread.currentThread().interrupt();
      }   

      buffer = mem.get_next_frame(handle);
    }

    System.out.println((char) buffer.get(0));
    System.out.println((char) buffer.get(1));
    System.out.println((char) buffer.get(2));
    System.out.println((char) buffer.get(3));
  }
}
