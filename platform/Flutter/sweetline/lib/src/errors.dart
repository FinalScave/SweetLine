part of '../sweetline.dart';

enum SweetLineErrorCode {
  ok,
  handleInvalid,
  jsonPropertyMissed,
  jsonPropertyInvalid,
  patternInvalid,
  stateInvalid,
  jsonInvalid,
  fileIoError,
  fileEmpty,
  unknown,
}

class SweetLineException implements Exception {
  SweetLineException(this.message, {this.code = SweetLineErrorCode.unknown});

  final String message;
  final SweetLineErrorCode code;

  @override
  String toString() {
    if (code == SweetLineErrorCode.unknown) {
      return 'SweetLineException: $message';
    }
    return 'SweetLineException($code): $message';
  }
}

class SyntaxCompileError extends SweetLineException {
  SyntaxCompileError(super.message, {super.code});

  @override
  String toString() {
    if (code == SweetLineErrorCode.unknown) {
      return 'SyntaxCompileError: $message';
    }
    return 'SyntaxCompileError($code): $message';
  }
}
