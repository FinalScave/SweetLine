# Smali sample file
# Android Dalvik bytecode in Smali format

.class public Lcom/example/demo/MainActivity;
.super Landroid/app/Activity;
.source "MainActivity.java"

# annotations
.annotation system Ldalvik/annotation/MemberClasses;
    value = {
        Lcom/example/demo/MainActivity$InnerClass;
    }
.end annotation

# static fields
.field private static final TAG:Ljava/lang/String; = "MainActivity"

# instance fields
.field private counter:I

.field private message:Ljava/lang/String;

# direct methods
.method static constructor <clinit>()V
    .registers 1

    const-string v0, "MainActivity"

    sput-object v0, Lcom/example/demo/MainActivity;->TAG:Ljava/lang/String;

    return-void
.end method

.method public constructor <init>()V
    .registers 2

    invoke-direct {p0}, Landroid/app/Activity;-><init>()V

    const/4 v0, 0x0

    iput v0, p0, Lcom/example/demo/MainActivity;->counter:I

    const-string v1, "Hello"

    iput-object v1, p0, Lcom/example/demo/MainActivity;->message:Ljava/lang/String;

    return-void
.end method

.method private calculate(II)I
    .registers 4

    add-int v0, p1, p2

    return v0
.end method

.method private factorial(I)I
    .registers 3

    const/4 v0, 0x1

    if-gt p1, v0, :cond_4

    return v0

    :cond_4
    mul-int/lit8 v0, p1, 0x1

    add-int/lit8 p1, p1, -0x1

    invoke-direct {p0, p1}, Lcom/example/demo/MainActivity;->factorial(I)I

    move-result p1

    mul-int/2addr v0, p1

    return v0
.end method

# virtual methods
.method protected onCreate(Landroid/os/Bundle;)V
    .registers 4

    invoke-super {p0, p1}, Landroid/app/Activity;->onCreate(Landroid/os/Bundle;)V

    const/high16 v0, 0x7f03

    invoke-virtual {p0, v0}, Lcom/example/demo/MainActivity;->setContentView(I)V

    const/high16 v0, 0x7f08

    invoke-virtual {p0, v0}, Lcom/example/demo/MainActivity;->findViewById(I)Landroid/view/View;

    move-result-object v0

    check-cast v0, Landroid/widget/TextView;

    iget-object v1, p0, Lcom/example/demo/MainActivity;->message:Ljava/lang/String;

    invoke-virtual {v0, v1}, Landroid/widget/TextView;->setText(Ljava/lang/CharSequence;)V

    return-void
.end method

.method public onClick(Landroid/view/View;)V
    .registers 4

    iget v0, p0, Lcom/example/demo/MainActivity;->counter:I

    add-int/lit8 v0, v0, 0x1

    iput v0, p0, Lcom/example/demo/MainActivity;->counter:I

    sget-object v0, Ljava/lang/System;->out:Ljava/io/PrintStream;

    new-instance v1, Ljava/lang/StringBuilder;

    invoke-direct {v1}, Ljava/lang/StringBuilder;-><init>()V

    const-string v2, "Counter: "

    invoke-virtual {v1, v2}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    iget v2, p0, Lcom/example/demo/MainActivity;->counter:I

    invoke-virtual {v1, v2}, Ljava/lang/StringBuilder;->append(I)Ljava/lang/StringBuilder;

    invoke-virtual {v1}, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;

    move-result-object v1

    invoke-virtual {v0, v1}, Ljava/io/PrintStream;->println(Ljava/lang/String;)V

    return-void
.end method

.method public testArray()V
    .registers 5

    const/4 v0, 0x5

    new-array v0, v0, [I

    fill-array-data v0, :array_0

    const/4 v1, 0x0

    :goto_0
    array-length v2, v0

    if-ge v1, v2, :cond_0

    aget v2, v0, v1

    sget-object v3, Ljava/lang/System;->out:Ljava/io/PrintStream;

    invoke-virtual {v3, v2}, Ljava/io/PrintStream;->println(I)V

    add-int/lit8 v1, v1, 0x1

    goto :goto_0

    :cond_0
    return-void

    :array_0
    .array-data 4
        0x1
        0x2
        0x3
        0x4
        0x5
    .end array-data
.end method

.method public testException()V
    .registers 3

    :try_start_0
    const/4 v0, 0x0

    const/4 v1, 0x1

    div-int/2addr v1, v0
    :try_end_0
    .catch Ljava/lang/ArithmeticException; {:try_start_0 .. :try_end_0} :catch_0

    goto :goto_0

    :catch_0
    move-exception v0

    sget-object v1, Ljava/lang/System;->out:Ljava/io/PrintStream;

    const-string v2, "Division by zero"

    invoke-virtual {v1, v2}, Ljava/io/PrintStream;->println(Ljava/lang/String;)V

    :goto_0
    return-void
.end method

.method public testInstanceOf(Ljava/lang/Object;)V
    .registers 3

    instance-of v0, p1, Ljava/lang/String;

    if-eqz v0, :cond_0

    check-cast p1, Ljava/lang/String;

    sget-object v0, Ljava/lang/System;->out:Ljava/io/PrintStream;

    invoke-virtual {v0, p1}, Ljava/io/PrintStream;->println(Ljava/lang/String;)V

    :cond_0
    return-void
.end method

.method public testSwitch(I)V
    .registers 3

    packed-switch p1, :pswitch_data_0

    sget-object v0, Ljava/lang/System;->out:Ljava/io/PrintStream;

    const-string v1, "Default"

    invoke-virtual {v0, v1}, Ljava/io/PrintStream;->println(Ljava/lang/String;)V

    goto :goto_0

    :pswitch_0
    sget-object v0, Ljava/lang/System;->out:Ljava/io/PrintStream;

    const-string v1, "One"

    invoke-virtual {v0, v1}, Ljava/io/PrintStream;->println(Ljava/lang/String;)V

    goto :goto_0

    :pswitch_1
    sget-object v0, Ljava/lang/System;->out:Ljava/io/PrintStream;

    const-string v1, "Two"

    invoke-virtual {v0, v1}, Ljava/io/PrintStream;->println(Ljava/lang/String;)V

    goto :goto_0

    :pswitch_2
    sget-object v0, Ljava/lang/System;->out:Ljava/io/PrintStream;

    const-string v1, "Three"

    invoke-virtual {v0, v1}, Ljava/io/PrintStream;->println(Ljava/lang/String;)V

    :goto_0
    return-void

    :pswitch_data_0
    .packed-switch 0x1
        :pswitch_0
        :pswitch_1
        :pswitch_2
    .end packed-switch
.end method

# Inner class
.class Lcom/example/demo/MainActivity$InnerClass;
.super Ljava/lang/Object;
.source "MainActivity.java"

.annotation system Ldalvik/annotation/EnclosingClass;
    value = Lcom/example/demo/MainActivity;
.end annotation

.annotation system Ldalvik/annotation/InnerClass;
    accessFlags = 0x0
    name = "InnerClass"
.end annotation

.field private value:I

.method public constructor <init>()V
    .registers 2

    invoke-direct {p0}, Ljava/lang/Object;-><init>()V

    const/4 v0, 0x0

    iput v0, p0, Lcom/example/demo/MainActivity$InnerClass;->value:I

    return-void
.end method

.method public getValue()I
    .registers 2

    iget v0, p0, Lcom/example/demo/MainActivity$InnerClass;->value:I

    return v0
.end method
