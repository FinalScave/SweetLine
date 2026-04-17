package com.qiplat.sweetline;

/**
 * Exception thrown when syntax rule compilation fails
 */
public class SyntaxCompileError extends Exception {
    public static final int ERR_JSON_PROPERTY_MISSED = -1;
    public static final int ERR_JSON_PROPERTY_INVALID = -2;
    public static final int ERR_PATTERN_INVALID = -3;
    public static final int ERR_STATE_INVALID = -4;
    public static final int ERR_JSON_INVALID = -5;
    public static final int ERR_FILE_NOT_EXISTS = -6;
    public static final int ERR_FILE_INVALID = -7;
    public static final int ERR_IMPORT_SYNTAX_NOT_FOUND = -8;
    public static final int ERR_STATE_REFERENCE_NOT_FOUND = -9;

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
