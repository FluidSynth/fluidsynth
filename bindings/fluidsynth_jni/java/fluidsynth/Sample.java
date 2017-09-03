package fluidsynth;

public class Sample
{
    protected int sampleNum = -1;
    protected int rootkey;
    protected String filename;

    public Sample(String filename, int rootkey) throws FluidException {
        sampleNum = newSample(filename, rootkey);
        if (sampleNum < 0) {
            throw new FluidException("Failed to load the sample (err=" + sampleNum + ")");
        }
        this.filename = filename;
        this.rootkey = rootkey;
    }

    protected void finalize() {
        if (sampleNum >= 0) {
            deleteSample(sampleNum);
            sampleNum = -1;
        }
    }

    public int getRootKey() {
        return rootkey;
    }

    public String getFileName() {
	return filename;
    }

    int getSampleNum() {
	return sampleNum;
    }

    protected native int newSample(String filename, int rootkey);
    protected native void deleteSample(int sampleNum);
}
