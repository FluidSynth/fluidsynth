package fluidsynth;

public class Synth
{
    protected int synth = -1;

    static {
        System.loadLibrary("fluidsynth_jni");
    }

    public Synth() throws FluidException {
        synth = newSynth();
        if (synth < 0) {
            throw new FluidException("Low-level initialization of the synthesizer failed");
        }
    }

    protected void finalize() {
        if (synth >= 0) {
            deleteSynth(synth);
            synth = -1;
        }
    }

    public void add(Sample sample, int bank, int preset, int lokey, int hikey) throws FluidException {
        if (add(synth, sample.getSampleNum(), bank, preset, lokey, hikey) != 0) {
            throw new FluidException("Failed to add the sample");
        }
    }

    public void remove(Sample sample, int bank, int preset) throws FluidException {
        if (remove(synth, sample.getSampleNum(), bank, preset) != 0) {
            throw new FluidException("Failed to remove the sample");
        }
    }

    public void loadSoundFont(String filename) throws FluidException {
        if (loadSoundFont(synth, filename) != 0) {
            throw new FluidException("Failed to load the SoundFont");
        }       
    }

    protected native int newSynth();
    protected native void deleteSynth(int synth);
    protected native int add(int synth, int sample, int bank, 
			     int preset, int lokey, int hikey);
    protected native int remove(int synth, int sample, int bank, int preset);
    protected native int loadSoundFont(int synth, String filename);
}
