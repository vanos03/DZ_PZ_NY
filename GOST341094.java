import org.bouncycastle.crypto.digests.GOST3411Digest;
import org.bouncycastle.jce.provider.BouncyCastleProvider;

import java.io.File;
import java.io.IOException;
import java.math.BigInteger;
import java.nio.file.Files;
import java.nio.file.StandardOpenOption;
import java.security.Security;

import static java.lang.Math.pow;

public class GOST341094 {
    private String inputFilePath;
    private String sigFilePath;
//    private String pqaFilePath;
//    private String pubkeyFilePath;
    public GOST341094(){
        this.inputFilePath = null;
        this.sigFilePath = null;
//        this.pqaFilePath = null;
//        this.pubkeyFilePath = null;
    }
    public GOST341094(String file){
        try {
            this.inputFilePath = file;
        } catch (Exception e){
            e.printStackTrace();
        }
    }
    public GOST341094(String infile, String sigfile){
        try {
            this.inputFilePath = infile;
            this.sigFilePath = sigfile;
        } catch (Exception e){
            e.printStackTrace();
        }
    }

    private static void writeToFile(File file, String data) {
        try {
            Files.write(file.toPath(), data.getBytes(), StandardOpenOption.CREATE);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static byte[] readFromFile(File file) {
        byte[] data = null;
        try {
            data = Files.readAllBytes(file.toPath());
        } catch (IOException e) {
            e.printStackTrace();
        }
        return data;
    }
    private static String toHexString(byte[] data) {
        StringBuilder sb = new StringBuilder();
        for (byte b : data) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public int subscribe_file() {
        try {
            Security.addProvider(new BouncyCastleProvider()); // Add Bouncy Castle as a security provider

            //Scanner scanner = new Scanner(System.in);
            GOST3411Digest gost3411 = new GOST3411Digest();

            LinearCongruentialGenerator g = new LinearCongruentialGenerator(
                    0x3DFC46F1,
                    97781173,
                    0xD,
                    (long) pow(2, 32)
            );

            Generator generator = new Generator(g);
            BigInteger[] primes = generator.generatePrimes1024();
            BigInteger p = primes[0];
            BigInteger q = primes[1];
            BigInteger a = generator.generateA(p, q);

            GOSTSignature ds = new GOSTSignature(p, q, a);
            int key_length = 256;
            BigInteger x = ds.getRandomPrivateKey(key_length);
            BigInteger y = ds.getPublicKey(x);

            File inputFile = new File(this.inputFilePath);
            byte[] message = readFromFile(inputFile);
            gost3411.update(message, 0, message.length);
            byte[] hash = new byte[gost3411.getDigestSize()];
            gost3411.doFinal(hash, 0);

            File fileSig = new File(this.inputFilePath + ".sig");

            String signature = ds.sign(toHexString(hash), x) + "\n";
            String dataWrite = p.toString(16) + "\n" + q.toString(16) + "\n" +
                    a.toString(16)+ "\n" + y.toString(16) + "\n" + signature + "\n";
            writeToFile(fileSig, dataWrite);

            return 0;
        } catch (Exception e) {
            e.printStackTrace();
            return -1;
        }
    }

    public int check_signature_file(){
        try {
            Security.addProvider(new BouncyCastleProvider());
            GOST3411Digest gost3411 = new GOST3411Digest();

//            System.out.println("File with p, q, a:");
            File sigFile = new File(this.sigFilePath);

            byte[] data = readFromFile(sigFile);
            String[] params = new String(data).split("\n");
            for (int i =0; i < 5; i++)
                System.out.println(params[i]);
            BigInteger p = new BigInteger(params[0], 16);
            BigInteger q = new BigInteger(params[1], 16);
            BigInteger a = new BigInteger(params[2], 16);

//            System.out.println("File with public key:");
//            File pubkeyfile = new File(this.pubkeyFilePath);
//            data = readFromFile(pubkeyfile);
            BigInteger y = new BigInteger(params[3], 16);
            GOSTSignature ds = new GOSTSignature(p, q, a);
//            File sigfile = new File(this.sigFilePath);
//            data = readFromFile(sigfile);
            String signature = new String(params[4]);

            File inputfile = new File(this.inputFilePath);
            data = readFromFile(inputfile);
            gost3411.update(data, 0, data.length);
            byte[] hash = new byte[gost3411.getDigestSize()];
            gost3411.doFinal(hash, 0);

//            System.out.println("File with signature:");

//            System.out.println("Hash:" + toHexString(hash));
//            System.out.println("Verification of a signature: " + signature);
            boolean result = ds.verify(toHexString(hash), signature, y);
            if (result) {
                return 0;
            } else {
                return 1;
            }
        } catch (Exception e) {
            e.printStackTrace();
            return -1;
        }
    }
}
