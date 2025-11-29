package com.qiplat.sweetline;

/**
 * Exception thrown when syntax rule compilation fails
 */
public class SyntaxCompileError extends Exception {

    private final int errorCode;

    public SyntaxCompileError(int errorCode, String message) {
        super(message);
        this.errorCode = errorCode;
    }

    /**
     * Get the error code
     *
     * @return Error code, see sl_error_t in c_sweetline.h
     */
    public int getErrorCode() {
        return errorCode;
    }
}
