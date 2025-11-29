package com.qiplat.sweetline;

public class SyntaxCompileError extends Exception {
    public SyntaxCompileError() {
    }

    public SyntaxCompileError(String message) {
        super(message);
    }

    public SyntaxCompileError(String message, Throwable cause) {
        super(message, cause);
    }
}
