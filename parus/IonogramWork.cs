using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Windows.Forms;

namespace parus
{
    // Информация о значениях в файле ионограммы.
    struct contentInfo {
        public int min_o;
        public int min_x;
        public int max_o;
        public int max_x;
        public bool read_error;
        public ushort[] values;
    } // contentInfo

    // Структура заголовка файла ионограммы.
    struct IonogramInfo2 {
        // GMT время получения ионограммы -----------------------------
        public int tm_sec;      //	seconds after the minute	0-60*
        public int tm_min;	    //	minutes after the hour	0-59
        public int tm_hour;	    //	hours since midnight	0-23
        public int tm_mday;	    //	day of the month	1-31
        public int tm_mon;	    //	months since January	0-11
        public int tm_year;	    //	years since 1900	
        public int tm_wday;	    //	days since Sunday	0-6
        public int tm_yday;	    //	days since January 1	0-365
        public int tm_isdst;	//	Daylight Saving Time flag
	    // ------------------------------------------------------------
        public uint height_min; // начальная высота, м (0 не означает, что зондирование от поверхности. Есть задержки!!!)
        public uint height_step; // шаг по высоте, м
        public uint count_height; // число высот
        public uint switch_frequency; // частота переключения антенн ионозонда 
        public uint freq_min; // начальная частота, кГц (первого модуля)
        public uint freq_max; // конечная частота, кГц (последнего модуля)   
        public uint count_freq; // число частот во всех модулях
        public uint count_modules; // количество модулей зондирования

        public string toString() {
            string tmp = "";
            //Console.WriteLine("Имя: {0}, возраст: {1}", Name, Age);

            return tmp;
        }
    } // Конец struct IonogramInfo2

    /* =====================================================================  */
    /* Родные структуры данных ИПГ-ИЗМИРАН */
    /* =====================================================================  */
    /* Каждая строка начинается с заголовка следующей структуры */
    struct FrequencyData {
        public ushort frequency; //!< Частота зондирования, [кГц].
        public ushort gain_control; // !< Значение ослабления входного аттенюатора дБ.
        public ushort pulse_time; //!< Время зондирования на одной частоте, [мс].
        public byte pulse_length; //!< Длительность зондирующего импульса, [мкc].
        public byte band; //!< Полоса сигнала, [кГц].
        public byte type; //!< Вид модуляции (0 - гладкий импульс, 1 - ФКМ).
        public byte threshold_o; //!< Порог амплитуды обыкновенной волны, ниже которого отклики не будут записываться в файл, [Дб/ед. АЦП].
        public byte threshold_x; //!< Порог амплитуды необыкновенной волны, ниже которого отклики не будут записываться в файл, [Дб/ед. АЦП].
        public byte count_o; //!< Число сигналов компоненты O.
        public byte count_x; //!< Число сигналов компоненты X.
    }

    /* Сначала следуют FrequencyData::count_o структур SignalResponse, описывающих обыкновенную волну. */
    /* Cразу же после перечисления всех SignalResponse  и SignalSample  для обыкновенных откликов следуют FrequencyData::count_x */
    /* структур SignalResponse, описывающих необыкновенные отклики с массивом структур SignalSample после каждой из них. Величины */
    /* FrequencyData::count_o и FrequencyData::count_x могут быть равны нулю, тогда соответствующие данные отсутствуют. */
    struct SignalResponse {
        public ulong height_begin; //!< начальная высота, [м]
        public ushort count_samples; //!< Число дискретов
    }

    class IonogramReader
    {
        // Поля
        private contentInfo _contentInfo; // информация о содержимом файла ионограммы
        private uint _ver; // номер версии формата ионограммы 0 - ИПГ, 1 - интенсивности без обработки, 2 - CDF
        private IonogramInfo2 _header; // заголовок ионограммы
        private string _filename; // путь к файлу ионограммы
        private Bitmap _image_o = null; // рисунок ионограммы o-след
        private Bitmap _image_x = null; // рисунок ионограммы x-след

        // Свойства
        public string FileName { get { return _filename; } }
        public uint Version { get { return _ver; } }
        public IonogramInfo2 Header { get { return _header; } }
        public Bitmap Bitmap_O { get { return _image_o; } }
        public Bitmap Bitmap_X { get { return _image_x; } }

        public string TimeString { get { 
            return (_header.tm_year+1900).ToString() + '-' + (_header.tm_mon+1).ToString("D2") + '-' +
                (_header.tm_mday).ToString("D2") + ' ' + (_header.tm_hour).ToString("D2") + ':' + (_header.tm_min).ToString("D2") + " UT"; 
        } }

        // Методы
        public IonogramReader(string fname)
        {
            this._filename = fname;

            try
            {
                // создаем объект BinaryReader
                using (BinaryReader reader = new BinaryReader(File.Open(fname, FileMode.Open)))
                {
                    _contentInfo = getContentInfo(reader);
                    _ver = reader.ReadUInt32();
                    switch (_ver)
                    {
                        case 0: // ИПГ
                            _header = readHeader2(reader);
                            _image_o = new Bitmap(
                                (int)_header.count_freq, 
                                (int)_header.count_height, 
                                System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                            _image_x = new Bitmap(_image_o);
                            fillImage(reader);
                            break;
                        case 1: // грязная ионограмма as is
                            _header = readHeader2(reader);
                            _image_o = new Bitmap(
                                (int)_header.count_freq,
                                (int)_header.count_height,
                                System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                            fillDirtyImage(reader);
                            //_image_x = new Bitmap(_image_o); // дублируем рисунок
                            break;
                        default:
                            throw new ArgumentOutOfRangeException("Неизвестная версия формата файла ионограмм.");
                    }
                }
            }
            catch (Exception e)
            {
                string caption = "Ошибка при чтении файла " + this._filename + '.';
                DialogResult result = MessageBox.Show(e.Message, caption, MessageBoxButtons.OK);
            }
        }

        protected IonogramInfo2 readHeader2(BinaryReader reader)
        {
            IonogramInfo2 tmp;

            // GMT время получения ионограммы -----------------------------
            tmp.tm_sec = reader.ReadInt32();      //	seconds after the minute	0-60*
            tmp.tm_min = reader.ReadInt32();	    //	minutes after the hour	0-59
            tmp.tm_hour = reader.ReadInt32();	    //	hours since midnight	0-23
            tmp.tm_mday = reader.ReadInt32();	    //	day of the month	1-31
            tmp.tm_mon = reader.ReadInt32();	    //	months since January	0-11
            tmp.tm_year = reader.ReadInt32();	    //	years since 1900	
            tmp.tm_wday = reader.ReadInt32();	    //	days since Sunday	0-6
            tmp.tm_yday = reader.ReadInt32();	    //	days since January 1	0-365
            tmp.tm_isdst = reader.ReadInt32();	//	Daylight Saving Time flag
	        // ------------------------------------------------------------
            tmp.height_min = reader.ReadUInt32(); // начальная высота, м (0 не означает, что зондирование от поверхности. Есть задержки!!!)
            tmp.height_step = reader.ReadUInt32(); // шаг по высоте, м
            tmp.count_height = reader.ReadUInt32(); // число высот
            tmp.switch_frequency = reader.ReadUInt32(); // частота переключения антенн ионозонда 
            tmp.freq_min = reader.ReadUInt32(); // начальная частота, кГц (первого модуля)
            tmp.freq_max = reader.ReadUInt32(); // конечная частота, кГц (последнего модуля)   
            tmp.count_freq = reader.ReadUInt32(); // число частот во всех модулях
            tmp.count_modules = reader.ReadUInt32(); // количество модулей зондирования

            return tmp;
        }

        protected FrequencyData readFrequencyData(BinaryReader reader)
        {
            FrequencyData tmp = new FrequencyData();
        
            tmp.frequency = reader.ReadUInt16(); //!< Частота зондирования, [кГц].
            tmp.gain_control = reader.ReadUInt16(); // !< Значение ослабления входного аттенюатора дБ.
            tmp.pulse_time = reader.ReadUInt16(); //!< Время зондирования на одной частоте, [мс].
            tmp.pulse_length = reader.ReadByte(); //!< Длительность зондирующего импульса, [мкc].
            tmp.band = reader.ReadByte(); //!< Полоса сигнала, [кГц].
            tmp.type = reader.ReadByte(); //!< Вид модуляции (0 - гладкий импульс, 1 - ФКМ).
            tmp.threshold_o = reader.ReadByte(); //!< Порог амплитуды обыкновенной волны, ниже которого отклики не будут записываться в файл, [Дб/ед. АЦП].
            tmp.threshold_x = reader.ReadByte(); //!< Порог амплитуды необыкновенной волны, ниже которого отклики не будут записываться в файл, [Дб/ед. АЦП].
            tmp.count_o = reader.ReadByte(); //!< Число сигналов компоненты O.
            tmp.count_x = reader.ReadByte(); //!< Число сигналов компоненты X.
            
            return tmp;
        }

        protected SignalResponse readSignalResponse(BinaryReader reader){
            SignalResponse tmp = new SignalResponse();

            tmp.height_begin = reader.ReadUInt32(); //!< начальная высота, [м]
            tmp.count_samples = reader.ReadUInt16(); //!< Число дискретов

            return tmp;
        }

        protected void fillImage(BinaryReader reader)
        {
            FrequencyData curFrq;
            SignalResponse curSignal;
            int i, iFrq;
            int df = (int)((_header.freq_max - _header.freq_min) / (_header.count_freq - 1));
            byte[] samples;

            // пока не достигнут конец файла считываем каждое значение из файла
            bool file_eof;
            try
            {
                do
                {
                    curFrq = readFrequencyData(reader);
                    iFrq = (int)((curFrq.frequency - _header.freq_min) / df);
                    for (i = 0; i < curFrq.count_o; i++) // о-отражения
                    {
                        curSignal = readSignalResponse(reader);
                        samples = reader.ReadBytes(curSignal.count_samples);
                        fillLine(curFrq, curSignal, samples, 0, iFrq);
                    }
                    for (i = 0; i < curFrq.count_x; i++) // x-отражения
                    {
                        curSignal = readSignalResponse(reader);
                        samples = reader.ReadBytes(curSignal.count_samples);
                        fillLine(curFrq, curSignal, samples, 1, iFrq);
                    }
                    file_eof = reader.BaseStream.Position >= reader.BaseStream.Length;
                }
                while (!file_eof); // если возникает ошибка - файл битый
            }
            catch (IOException e)
            {
                string caption = "Ошибка чтения файла ионограммы";
                DialogResult result = MessageBox.Show(e.Message, caption, MessageBoxButtons.OK);
            }
        }

        protected void fillDirtyImage(BinaryReader reader)
        {
            int i, j;
            uint sample;
            // ushort sample;
            Color cl;

            try
            {
                for (i = 0; i < _header.count_freq; i++)
                    for (j = 0; j < _header.count_height; j++)
                    {
                        sample = _contentInfo.values[i * _header.count_height+j];
                        cl = MapRainbowColor(sample, _contentInfo.max_o, _contentInfo.min_o);
                        _image_o.SetPixel(i, (int)_header.count_height - 1 - j, cl);
                    }
            }
            catch (IOException e)
            {
                string caption = "Ошибка чтения файла ионограммы";
                DialogResult result = MessageBox.Show(e.Message, caption, MessageBoxButtons.OK);
            }
        }

        protected void fillLine(FrequencyData curFrq, SignalResponse curSignal, byte[] samples, byte component, int i)
        {
            // component - 0 для о-компоненты, 1 для х-компоненты
            // i - номер частоты (ось Х)
            int j, j_beg, k;

            if (!_contentInfo.read_error)
            {
                j_beg = (int)((curSignal.height_begin - _header.height_min) / _header.height_step); // j_beg - начальный номер высоты (ось Y) 
                for (k = 0; k < curSignal.count_samples; k++)
                {
                    j = (int)_header.count_height - 1 - (j_beg + k);
                    switch (component)
                    {
                        case 0: // красным рисуем о-компоненту
                            _image_o.SetPixel(i, j, MapRainbowColor(samples[k], _contentInfo.max_o, _contentInfo.min_o));
                            break;
                        case 1: // зеленым рисуем х-компоненту
                            _image_x.SetPixel(i, j, MapRainbowColor(samples[k], _contentInfo.max_x, _contentInfo.min_x));
                            break;
                    }
                }
            }
        }

        protected contentInfo getContentInfo(BinaryReader reader)
        {
            contentInfo _out;
            int i, k;
            byte[] samples;
            ushort[] hugeUShortArray = null;

            // Инициализация
            _out.min_o = 255;
            _out.min_x = 255;
            _out.max_o = 0;
            _out.max_x = 0;
            _out.read_error = false;
            _out.values = hugeUShortArray;

            // Сохраним положение в файле.
            long oldPos = reader.BaseStream.Position;

            try
            {
                // Читаем заголовок.
                _ver = reader.ReadUInt32();
                if (_ver >= 0 && _ver <=2)
                    _header = readHeader2(reader);
                else
                    throw new ArgumentOutOfRangeException("Неизвестная версия формата файла ионограмм.");

                int BufferSize = (int)this._header.count_height * (int)this._header.count_freq;
                hugeUShortArray = new ushort[BufferSize];
                    
                bool file_eof = false;
                switch(_ver) 
                {
                    case 0: // ИПГ
                    // Пробегаем по данным.
                    FrequencyData curFrq;
                    SignalResponse curSignal;

                    // пока не достигнут конец файла считываем каждое значение из файла

                    do
                    {
                        curFrq = readFrequencyData(reader);
                        for (i = 0; i < curFrq.count_o; i++) // о-отражения
                        {
                            curSignal = readSignalResponse(reader);
                            samples = reader.ReadBytes(curSignal.count_samples);
                            for (k = 0; k < curSignal.count_samples; k++)
                            {
                                _out.min_o = (_out.min_o < samples[k]) ? _out.min_o : samples[k];
                                _out.max_o = (_out.max_o > samples[k]) ? _out.max_o : samples[k];
                            }
                        }
                        for (i = 0; i < curFrq.count_x; i++) // о-отражения
                        {
                            curSignal = readSignalResponse(reader);
                            samples = reader.ReadBytes(curSignal.count_samples);
                            for (k = 0; k < curSignal.count_samples; k++)
                            {
                                _out.min_x = (_out.min_x < samples[k]) ? _out.min_x : samples[k];
                                _out.max_x = (_out.max_x > samples[k]) ? _out.max_x : samples[k];
                            }
                        }
                        file_eof = reader.BaseStream.Position >= reader.BaseStream.Length;
                    }
                    while (!file_eof); // если возникает ошибка - файл битый
                    break;

                    case 1: // грязная ионограмма

                    // превести uint к ushort и вместо 4х поставить 2              
                    samples = reader.ReadBytes(2 * BufferSize);
                    Buffer.BlockCopy(samples, 0, hugeUShortArray, 0, 2 * BufferSize);

                    ushort sample;
                    for (i = 0; i < BufferSize; i++)
                    {
                        sample = hugeUShortArray[i];
                        _out.min_o = (_out.min_o < sample) ? _out.min_o : sample;
                        _out.max_o = (_out.max_o > sample) ? _out.max_o : sample;
                    }
                    _out.min_x = _out.min_o;
                    _out.max_x = _out.max_o;
                    _out.values = hugeUShortArray;

                    break;
                }
            }
            catch (IOException e)
            {
                // Ошибку не обрабатываем. Что нашли в файле - то нашли.
                _out.read_error = true;
            }

            // Возвратим исходное положение в файле.
            reader.BaseStream.Seek(oldPos, SeekOrigin.Begin);

            return _out;
        }

        // Map a value to a rainbow color.
        // red_value - max bound, blue_value - min bound (или наоборот)
        // value - между этими границами (реально от 0 до 255)
        private Color MapRainbowColor(float value, float red_value = 255, float blue_value = 0)
        {
            // Convert into a value between 0 and 255.
            int int_value = (int)(255 * (value - red_value) /
                (blue_value - red_value));

            // Map different color bands.
            if (int_value < 64)
            {
                // Red to yellow. (255, 0, 0) to (255, 255, 0).
                return Color.FromArgb(128, 255, int_value, 0);
            }
            else if (int_value < 128)
            {
                // Yellow to green. (255, 255, 0) to (0, 255, 0).
                int_value -= 64;
                return Color.FromArgb(128, 255 - int_value, 255, 0);
            }
            else if (int_value < 192)
            {
                // Green to aqua. (0, 255, 0) to (0, 255, 255).
                int_value -= 128;
                return Color.FromArgb(128, 0, 255, int_value);
            }
            else
            {
                // Aqua to blue. (0, 255, 255) to (0, 0, 255).
                int_value -= 192;
                return Color.FromArgb(128, 0, 255 - int_value, 255);
            }
        }
    }
}
