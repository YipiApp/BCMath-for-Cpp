BC-Math-Qt5
===========

BC-Math PHP (https://php.net/manual/en/intro.bc.php) Adapted to Qt. Tested on QT5.


BC Math Functions
===========

- QBCMath::bcadd: Add two arbitrary precision numbers (https://php.net/manual/en/function.bcadd.php)
    
- QBCMath::bccomp: Compare two arbitrary precision numbers (https://php.net/manual/en/function.bccomp.php)
    
- QBCMath::bcdiv: Divide two arbitrary precision numbers (https://php.net/manual/en/function.bcdiv.php)
    
- QBCMath::bcmod: Get modulus of an arbitrary precision number (https://php.net/manual/en/function.bcmod.php)
    
- QBCMath::bcmul: Multiply two arbitrary precision numbers (https://php.net/manual/en/function.bcmul.php)
    
- QBCMath::bcpow: Raise an arbitrary precision number to another (https://php.net/manual/en/function.bcpow.php)
    
- QBCMath::bcscale: Set default scale parameter for all bc math functions (https://php.net/manual/en/function.bcscale.php)
    
- QBCMath::bcsub: Subtract one arbitrary precision number from another (https://php.net/manual/en/function.bcsub.php)

- QBCMath::bcround: Round decimals, receives a single parameter to determine how many decimal places should be.
    
    

Funciones de BC Math
===========

- QBCMath::bcadd: Añade dos números de precisión arbitrária (https://php.net/manual/es/function.bcadd.php)
    
- QBCMath::bccomp: Compara dos números de precisión arbitraria (https://php.net/manual/es/function.bccomp.php)
    
- QBCMath::bcdiv: Divide dos números de precisión arbitraria (https://php.net/manual/es/function.bcdiv.php)
    
- QBCMath::bcmod: Obtiene el módulo de un número de precisión arbitraria (https://php.net/manual/es/function.bcmod.php)
    
- QBCMath::bcmul: Multiplica dos números de precisión arbitraria (https://php.net/manual/es/function.bcmul.php)
    
- QBCMath::bcpow: Elevar un número de precisión arbitraria a otro (https://php.net/manual/es/function.bcpow.php)
    
- QBCMath::bcscale: Establece los parametros de scale por defecto para todas las funciones matemáticas de bc (https://php.net/manual/es/function.bcscale.php)
    
- QBCMath::bcsub: Resta un número de precisión arbitraria de otro (https://php.net/manual/es/function.bcsub.php)

- QBCMath::bcround: Redondea los decimales, recibe un solo parámetro para determinar cuantos decimales deben quedar.
    
USAGE:
===========

    #include "bcmath.h"

Code Test
===========
    
    QBCMath::bcscale(4); //Num Decimals
    QBCMath test("-5978");
    
    test^=30; //Pow, only integers. Not work decimals.
    qDebug()<<"Result BigDecimal 1: "<<test.toString().toStdString().c_str();
    
    test-=1.23; //sub
    qDebug()<<"Result BigDecimal 2: "<<test.toString().toStdString().c_str();
    
    test*=1.23; //mul
    qDebug()<<"Result BigDecimal 3: "<<test.toString().toStdString().c_str();
    
    test*=-1.23; //mul
    qDebug()<<"Result BigDecimal 4: "<<test.toString().toStdString().c_str();

    QBCMath::bcscale(70); //Num Decimals
    
    QBCMath randNum("-5943534512345234545.8998928392839247844353457");
    QBCMath pi("3.1415926535897932384626433832795028841971693993751058209749445923078164062862");

    QBCMath result1 = randNum + pi;
    QBCMath result2 = randNum - pi;
    QBCMath result3 = randNum * pi;
    QBCMath result4 = randNum / pi;

    qDebug()<<"Result Super Precision 1: "<<result1.toString().toStdString().c_str();
    qDebug()<<"Result Super Precision 2: "<<result2.toString().toStdString().c_str();
    qDebug()<<"Result Super Precision 3: "<<result3.toString().toStdString().c_str();
    qDebug()<<"Result Super Precision 4: "<<result4.toString().toStdString().c_str();
    
    //Other example
    QBCMath::bcscale(4); //Num Decimals
    qDebug()<<"Other 1: "<<QBCMath::bcmul("1000000.0134", "8.0234").toStdString().c_str();
    qDebug()<<"Other 2: "<<QBCMath::bcadd("1000000.0134", "8.0234").toStdString().c_str();
     
    qDebug()<<"Compare 1:  "<<QBCMath::bccomp("1", "2");
    qDebug()<<"Compare 2:  "<<QBCMath::bccomp("1.00001", "1", 3); 
    qDebug()<<"Compare 3:  "<<QBCMath::bccomp("1.00001", "1", 5);
    qDebug()<<"Compare 4:  "<<QBCMath("1")< QBCMath("2");
    qDebug()<<"Compare 5:  "<<QBCMath("1")<=QBCMath("2");
    qDebug()<<"Compare 6:  "<<QBCMath("1")> QBCMath("2");
    qDebug()<<"Compare 7:  "<<QBCMath("1")>=QBCMath("2");
    qDebug()<<"Compare 8:  "<<QBCMath("2")< QBCMath("2");
    qDebug()<<"Compare 9:  "<<QBCMath("2")<=QBCMath("2");
    qDebug()<<"Compare 10: "<<QBCMath("2")> QBCMath("2");
    qDebug()<<"Compare 11: "<<QBCMath("2")>=QBCMath("2");
    
    qDebug()<<"Round 1: "<<QBCMath::bcround("123.01254").toStdString().c_str();
    qDebug()<<"Round 2: "<<QBCMath::bcround("-123.01254", 3).toStdString().c_str();
    qDebug()<<"Round 3: "<<QBCMath::bcround("123.01254", 2).toStdString().c_str();
    pi.round(3);
    qDebug()<<"Round 4: "<<pi.toString().toStdString().c_str();

    QBCMath part1("-.123");
    QBCMath part2(".123");
    QBCMath part3("123");
    qDebug()<<"Int part 1: "<<part1.getIntPart().toStdString().c_str();
    qDebug()<<"Dec part 1: "<<part1.getDecPart().toStdString().c_str();
    qDebug()<<"Int part 2: "<<part2.getIntPart().toStdString().c_str();
    qDebug()<<"Dec part 2: "<<part2.getDecPart().toStdString().c_str();
    qDebug()<<"Int part 3: "<<part3.getIntPart().toStdString().c_str();
    qDebug()<<"Dec part 3: "<<part3.getDecPart().toStdString().c_str();


Result
===========

    Result BigDecimal 1:  198005530669253749533290222782634796336450786581284861381777714804795900171726938603997395193921984842256586113024
    Result BigDecimal 2:  198005530669253749533290222782634796336450786581284861381777714804795900171726938603997395193921984842256586113022.7700
    Result BigDecimal 3:  243546802723182111925946974022640799493834467494980379499586589209898957211224134482916796088524041355975600919018.0071
    Result BigDecimal 4:  -299562567349513997668914778047848183377416395018825866784491504728175717369805685413987659188884570867849989130392.1487

    Result Super Precision 1:  -5943534512345234542.7583001856941315459727023167204971158028306006248941790250554076921835
    Result Super Precision 2:  -5943534512345234549.0414854928737180228979890832795028841971693993751058209749445923078164
    Result Super Precision 3:  -18672164360341183116.9114783895073349180904753962992796943871920962352436079118338887287186
    Result Super Precision 4:  -1891885794154043400.2804849527556211973567525043250278948318788149660700494315139982452600
    
    Other 1:  8023400.1075
    Other 2:  1000008.0368
    
    Compare 1:   -1
    Compare 2:   0
    Compare 3:   1
    Compare 4:   true
    Compare 5:   true
    Compare 6:   false
    Compare 7:   false
    Compare 8:   false
    Compare 9:   true
    Compare 10:  false
    Compare 11:  true
    
    Round 1:  123.0125
    Round 2:  -123.013
    Round 3:  123.01
    Round 4:  3.142
    
    Int part 1:  -0
    Dec part 1:  123
    Int part 2:  0
    Dec part 2:  123
    Int part 3:  123
    Dec part 3:  0