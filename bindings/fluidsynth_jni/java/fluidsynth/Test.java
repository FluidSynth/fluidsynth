package fluidsynth;
import java.io.*;

public class Test
{
    static public void main(String[] argv) throws Exception {
        try{
            Synth synth = new Synth();
            synth.add(new Sample("[accord guitar].wav", 55), 0, 0, 50,58);
            
            synth.add(new Sample("cabrel.wav_14.wav", 60), 0, 0, 59,59);
            synth.add(new Sample("cabrel.wav_16.wav", 60), 0, 0, 60,60);
            synth.add(new Sample("cabrel.wav_15.wav", 62), 0, 0, 61, 61);
            synth.add(new Sample("cabrel.wav_17.wav", 62), 0, 0, 62, 62);
            synth.add(new Sample("[TIR].wav", 64), 0, 0, 63,63);
            synth.add(new Sample("cabrel.wav_18.wav", 64), 0, 0, 64,64);
            synth.add(new Sample("cabrel.wav_5.wav", 64), 0, 0, 65,65);
            synth.add(new Sample("cabrel.wav_7.wav", 64), 0, 0, 66,66);
            synth.add(new Sample("cabrel.wav_8.wav", 64), 0, 0, 67,67);
            synth.add(new Sample("cabrel.wav_19.wav", 64), 0, 0, 68,68);
            synth.add(new Sample("cabrel.wav_9.wav", 65), 0, 0, 69,69);
            synth.add(new Sample("cabrel.wav_10.wav", 67), 0, 0, 70, 70);
            synth.add(new Sample("cabrel.wav_11.wav", 69), 0, 0, 71, 71);
            
            
            waitCR();            
        }catch(Exception e){System.out.println(e);waitCR();}
    }
    
    public static void waitCR() {
        System.out.println ("Appuyez sur CR ...");
        try {System.in.read();}
        catch (IOException e) {System.out.println(e);}
    }
}

