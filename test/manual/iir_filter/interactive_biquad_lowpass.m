#
# This Matlab script implements an interactive Bode plot of fluidsynth's IIR filter.
# To run it, just call interactive_biquad_lowpass() and a window will open up, allowing
# you to adjust the cutoff frequency and Q.
# Note that Q is in linear range here!
#
function interactive_biquad_lowpass()
    % Initial values
    f_c = 1000;  % Initial cutoff frequency in Hz
    Q = 0.707;   % Initial quality factor
    f_s = 48000; % Sampling frequency in Hz

    % Create the figure
    hFig = figure('Name', 'Interactive Biquad Lowpass Filter', 'NumberTitle', 'off');

    % Create axes for the magnitude and phase plots
    hAxesMag = subplot(2, 1, 1, 'Parent', hFig);
    hAxesPhase = subplot(2, 1, 2, 'Parent', hFig);

    % Plot the initial response
    plot_response(hAxesMag, hAxesPhase, f_c, Q, f_s);

    % Create slider for cutoff frequency
    uicontrol('Style', 'text', 'Position', [20 20 150 20], 'String', 'Cutoff Frequency (Hz)');
    hSliderFc = uicontrol('Style', 'slider', 'Min', 100, 'Max', 20000, 'Value', f_c, ...
                          'Position', [20 40 300 20]);
    hTextFc = uicontrol('Style', 'text', 'Position', [330 40 50 20], 'String', num2str(f_c));

    % Create slider for quality factor
    uicontrol('Style', 'text', 'Position', [20 80 150 20], 'String', 'Quality Factor (Q)');
    hSliderQ = uicontrol('Style', 'slider', 'Min', 0.1, 'Max', 100, 'Value', Q, ...
                         'Position', [20 100 300 20]);
    hTextQ = uicontrol('Style', 'text', 'Position', [330 100 50 20], 'String', num2str(Q));

    % Add listeners for both sliders
    addlistener(hSliderFc, 'Value', 'PreSet', @(src, event) update_plot(hAxesMag, hAxesPhase, hSliderFc, hSliderQ, f_s, hTextFc, hTextQ));
    addlistener(hSliderQ, 'Value', 'PreSet', @(src, event) update_plot(hAxesMag, hAxesPhase, hSliderFc, hSliderQ, f_s, hTextFc, hTextQ));
end

function update_plot(hAxesMag, hAxesPhase, hSliderFc, hSliderQ, f_s, hTextFc, hTextQ)
    % Get the current values from the sliders
    f_c = get(hSliderFc, 'Value');
    Q = get(hSliderQ, 'Value');

    % Update the text displays
    set(hTextFc, 'String', num2str(f_c, '%.1f'));  % Display cutoff frequency
    set(hTextQ, 'String', num2str(Q, '%.2f'));      % Display quality factor

    % Update the plot with the new values
    plot_response(hAxesMag, hAxesPhase, f_c, Q, f_s);
end

function plot_response(hAxesMag, hAxesPhase, f_c, Q, f_s)
    % Design the biquad lowpass filter
    w0 = 2 * pi * f_c / f_s;
    alpha = sin(w0) / (2 * Q);

    b0 = (1 - cos(w0)) / 2;
    b1 = 1 - cos(w0);
    b2 = (1 - cos(w0)) / 2;
    a0 = 1 + alpha;
    a1 = -2 * cos(w0);
    a2 = 1 - alpha;

    % Normalize coefficients
    b = [b0 / a0, b1 / a0, b2 / a0];
    a = [1, a1 / a0, a2 / a0];

    % Compute the frequency response
    [h, w] = freqz(b, a, 1024, f_s);

    % Clear the axes and plot the new response
    cla(hAxesMag);
    plot(hAxesMag, w, 20*log10(abs(h)));
    title(hAxesMag, 'Magnitude Response');
    xlabel(hAxesMag, 'Frequency (Hz)');
    ylabel(hAxesMag, 'Magnitude (dB)');
    grid(hAxesMag, 'on');

    cla(hAxesPhase);
    plot(hAxesPhase, w, angle(h) * (180/pi));
    title(hAxesPhase, 'Phase Response');
    xlabel(hAxesPhase, 'Frequency (Hz)');
    ylabel(hAxesPhase, 'Phase (degrees)');
    grid(hAxesPhase, 'on');
end
