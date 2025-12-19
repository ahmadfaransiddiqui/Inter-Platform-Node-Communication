import java.io.*;
import java.net.*;
import java.util.HashMap;
import java.util.Map;

public class NodeB2 {
    enum CacheState { INVALID, SHARED, MODIFIED }

    private static Map<Integer, CacheBlock> cache = new HashMap<>();

    static class CacheBlock {
        String data;
        CacheState state;

        CacheBlock() {
            this.data = null;
            this.state = CacheState.INVALID;
        }
    }

    public static void main(String[] args) {
        int port = 12345;
        String serverIP = "192.168.226.22"; // Replace with Node A's IP address
        int timeoutSeconds = 5; // Timeout duration in seconds

        try (Socket clientSocket = new Socket(serverIP, port);
             BufferedReader userInput = new BufferedReader(new InputStreamReader(System.in));
             BufferedReader reader = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
             PrintWriter writer = new PrintWriter(clientSocket.getOutputStream(), true)) {

            clientSocket.setSoTimeout(timeoutSeconds * 1000);

            System.out.println("Connected to Node A");

            while (true) {
                displayMenu();
                System.out.print("Enter your choice: ");
                String choice = userInput.readLine().toLowerCase();

                if (choice.equals("exit")) {
                    writer.println("exit");
                    break;
                } else {
                    writer.println(choice);

                    if (choice.equals("write")) {
                        System.out.print("Enter cache block index: ");
                        int blockIndex = Integer.parseInt(userInput.readLine());
                        System.out.print("Enter data to write: ");
                        String data = userInput.readLine();
                        writer.println("write " + blockIndex + " " + data);

                        String ackMessage = reader.readLine();
                        System.out.println("ackMessage: " + ackMessage);
                        if (ackMessage == null) {
                            System.out.println("Received a null acknowledgment message.");
                        } else if (ackMessage.startsWith("acknowledgment"+ blockIndex)) {
                            System.out.println("Received: " + ackMessage);

                            
    // Wait for the acknowledgment before sending the next command
    //writer.println("write " + blockIndex + " " + data);
    String nextCommandResponse = reader.readLine();
    if (nextCommandResponse != null && nextCommandResponse.startsWith("acknowledgment")) {
        System.out.println("Write operation acknowledged.");
        //writer.println("invalidate " + blockIndex);
    } else {
        System.out.println("Error in sending the next command.");
    }
    
                        } else {
                            System.out.println("Write operation acknowledged.");
                            System.out.println("Write operation failed.");
                            
                        }
                    } else if (choice.equals("invalidate")) {
                        System.out.print("Enter cache block index to invalidate: ");
                        int blockIndex = Integer.parseInt(userInput.readLine());

                        writer.println("acknowledgment " + blockIndex);

                        String invalidationMessage = reader.readLine();
                        if (invalidationMessage != null && invalidationMessage.startsWith("invalidate")) {
                            int invalidatedBlockIndex = Integer.parseInt(invalidationMessage.split(" ")[1]);
                            cache.get(invalidatedBlockIndex).state = CacheState.INVALID;
                            System.out.println("Cache block " + invalidatedBlockIndex + " invalidated.");
                        }
                    } else if (choice.equals("read")) {
                        System.out.print("Enter cache block index to read: ");
                        int blockIndex = Integer.parseInt(userInput.readLine());

                        // Send a read request to Node A
                        writer.println("read " + blockIndex);

                        // Wait for the response from Node A
                        String response = reader.readLine();
                        if (response != null) {
                            if (response.startsWith("data")) {
                                String data = response.split(" ")[1];
                                System.out.println("Data read from cache block " + blockIndex + ": " + data);
                            } else {
                                System.out.println("Read operation failed: " + response);
                            }
                        } else {
                            System.out.println("Read operation failed.");
                        }
                        // Implement read operation here if needed
                    } else {
                        System.out.println("Invalid choice. Please try again.");
                    }
                }
            }
        } catch (SocketTimeoutException e) {
            System.out.println("Socket timeout: Connection to the server has timed out.");
            // Handle the timeout here, such as re-establishing the connection or exiting gracefully.
        } catch (IOException e) {
            e.printStackTrace();
        }

        System.out.println("Disconnected from Node A");
    }

    private static void displayMenu() {
        System.out.println("\nMenu:");
        System.out.println("1. Read data from shared memory");
        System.out.println("2. Write data to shared memory");
        System.out.println("3. Invalidate cache block");
        System.out.println("4. Exit\n");
    }
}
