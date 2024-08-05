
1) ADC_PWM_069 Dosyasında
   -Bu proje, TMS320F28069m işlemcisini kullanarak analog sinyalleri okuyup dijital sinyallere çevirme (ADC) işlemini gerçekleştirmeyi amaçlamaktadır.
   Dönüştürülen dijital sinyali PWM sinyal yapılandırması yaparak PWM sinyali de üretmektedir.
   İşlevler;
      - Analog Sinyal Okuma
      - ADC(Analog to Digital Converter)
      - PWM Sinyali oluşturma
      - Timer kullanımı

2) PWM_069 Dosyasında
   - Bu proje, TMS320F28069m işlemcisini kullanarak sabit bir PWM yapılandırması yapıp sabit bir PWM sinyal değeri göndermeyi amaçlamaktadır.
   İşlevler;
      - PWM Sinyali oluşturma
      - Timer kullanımı

3) blink_069 Dosyasında
   - Bu proje, TMS320F28069m işlemcisini kullanarak sabit bir Dijital sinyal gönderir.
   İşlevler;
      - Dijital çıkış sinyali
    
4) blink_timer_069 Dosyasında
   - Bu proje, TMS320F28069m işlemcisini kullanarak sabit bir Dijital sinyal gönderir, bekleme süresi için delay yerine Timer kullanımı.
   İşlevler;
      - Dijital çıkış sinyali
      - Timer kullanımı

5) button_blink_Multiple_069 Dosyasında
   - Bu Proje TMS320F28069m işlemcisini kullanarak Dijital giriş değeri aldığında Dijital Çıkış sinyali vermektedir.
   İşlevler;
      - Dijital giriş sinyali
      - Dijital çıkış sinyali
      - Çoklu giriş-çıkış sinyali
     
6) button_blink_Multiple_Interrupts_069 Dosyasında
   - Bu Proje TMS320F28069m işlemcisini kullanarak Dijital giriş değeri aldığında Dijital Çıkış sinyali vermektedir. İşlemcinin işlemlerini Interrupt kullanarak çalışmasını sağlar.
   İşlevler;
      - Dijital giriş sinyali
      - Dijital çıkış sinyali
      - Çoklu giriş-çıkış sinyali
      - Interrupt kullanımı        
    
7) button_blink_timer_2_069 Dosyasında
   - Bu Proje TMS320F28069m işlemcisini kullanarak Dijital giriş değeri aldığında Dijital Çıkış sinyali vermektedir.
   Bekleme süresini ise delay kullanarak değil timer kullanarak yapmaktadır.
   İşlevler;
      - Dijital giriş sinyali
      - Dijital çıkış sinyali
      - Çoklu giriş-çıkış sinyali
      - Timer kullanımı
    
8) ADS1231_069 Dosyasında
   - Bu proje, TMS320F28069m işlemcisini kullanarak ADS1231 ADC entegresine dijital saat (clock) sinyali göndermektedir. ADS1231 entegresi, bu sinyale karşılık olarak 24 bitlik bir dijital çıkış (DOUT) sinyali üretir.
   - Projede, her 100 ms'de bir dijital saat sinyali gönderilir ve karşılığında 24 bitlik DOUT sinyali alınır. Bekleme süresi, gecikme (delay) kullanmak yerine timer ile kontrol edilmektedir.
   İşlevler;
      - Dijital giriş sinyali
      - Dijital çıkış sinyali
      - Timer kullanımı
      - ADS1231 ADC Entegresi
    
     
