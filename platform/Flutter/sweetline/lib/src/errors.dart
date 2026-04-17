part of '../sweetline.dart';

class SweetLineException implements Exception {
  SweetLineException(this.message, {this.errorCode});

  final String message;
  final int? errorCode;

  @override
  String toString() {
    if (errorCode == null) {
      return 'SweetLineException: $message';
    }
    return 'SweetLineException($errorCode): $message';
  }
}

class SyntaxCompileError extends SweetLineException {
  static const int errJsonPropertyMissed = -1;
  static const int errJsonPropertyInvalid = -2;
  static const int errPatternInvalid = -3;
  static const int errStateInvalid = -4;
  static const int errJsonInvalid = -5;
  static const int errFileNotExists = -6;
  static const int errFileInvalid = -7;
  static const int errImportSyntaxNotFound = -8;
  static const int errStateReferenceNotFound = -9;
  static const int errInlineStyleReferenceNotFound = -10;

  SyntaxCompileError(super.message, {required super.errorCode});

  @override
  String toString() {
    if (errorCode == null) {
      return 'SyntaxCompileError: $message';
    }
    return 'SyntaxCompileError($errorCode): $message';
  }
}
