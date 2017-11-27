using System;
using System.Drawing;
using System.Windows.Forms;
using System.Xml;


namespace parus
{
    // Перичисление доступных экспериментов
    public enum Measuring : int { ionogram, amplitudes }

    // Парсинг XML файла
    public class XMLConfig : XmlDocument
    {
        public int getMeasuringTime(Measuring mes)
        {
            int time = 0; // ms
            string s_attr = "";

            switch(mes)
            {
                case Measuring.amplitudes:
                    s_attr = "amplitudes";
                    break;
                case Measuring.ionogram:
                    s_attr = "ionogram";
                    break;
            }
                    
            // получим корневой элемент
            XmlElement xRoot = DocumentElement;
 
            // время для одного блока зондирования
            foreach(XmlNode xnode in xRoot)
            {
                // находим атрибут name
                if (xnode.NodeType == XmlNodeType.Element && xnode.Attributes.Count > 0)
                {
                    XmlNode attr = xnode.Attributes.GetNamedItem("name");
                    if (attr != null & attr.Value == s_attr)
                    {
                        XmlNode header_node = xnode["header"];
                        time = 1000 * Convert.ToInt32(header_node["pulse_count"].InnerText) / Convert.ToInt32(header_node["pulse_frq"].InnerText);

                        if (mes == Measuring.amplitudes) // умножим время измерения строки на количество частот
                        {
                            // нужно еще умножить на количество блоков !!!!!
                            int count = Convert.ToInt32(header_node["modules_count"].InnerText);
                            time *= count;
                        }

                        if (mes == Measuring.ionogram) // умножим время измерения строки на количество частот
                        {
                            XmlNode module = xnode["module"];
                            int fbeg = Convert.ToInt32(module["fbeg"].InnerText);
                            int fend = Convert.ToInt32(module["fend"].InnerText);
                            int fstep = Convert.ToInt32(module["fstep"].InnerText);
                            int count = 1 + (fend - fbeg) / fstep;
                            time *= count;
                        }
                    }
                }
            }

            return time;
        }
    }

    // Раскраска XML текста
    public class HighlightColors
    {
        public static Color HC_NODE = Color.Firebrick;
        public static Color HC_STRING = Color.Blue;
        public static Color HC_ATTRIBUTE = Color.Red;
        public static Color HC_COMMENT = Color.Green;
        public static Color HC_INNERTEXT = Color.Black;

        public static void HighlightRTF(RichTextBox rtb)
        {
            int k = 0;

            string str = rtb.Text;

            int st, en;
            int lasten = -1;
            while (k < str.Length)
            {
                st = str.IndexOf('<', k);
                if (st < 0) break;

                if (lasten > 0)
                {
                    rtb.Select(lasten + 1, st - lasten - 1);
                    rtb.SelectionColor = HighlightColors.HC_INNERTEXT;
                }

                en = str.IndexOf('>', st + 1);
                if (en < 0)
                    break;

                k = en + 1;
                lasten = en;

                if (str[st + 1] == '!')
                {
                    rtb.Select(st + 1, en - st - 1);
                    rtb.SelectionColor = HighlightColors.HC_COMMENT;
                    continue;
                }
                String nodeText = str.Substring(st + 1, en - st - 1);

                bool inString = false;
                int lastSt = -1;
                int state = 0;
                /* 0 = before node name
                 * 1 = in node name
                   2 = after node name
                   3 = in attribute
                   4 = in string
                   */
                int startNodeName = 0, startAtt = 0;
                for (int i = 0; i < nodeText.Length; ++i)
                {
                    if (nodeText[i] == '"')
                        inString = !inString;

                    if (inString && nodeText[i] == '"')
                        lastSt = i;
                    else
                        if (nodeText[i] == '"')
                        {
                            rtb.Select(lastSt + st + 2, i - lastSt - 1);
                            rtb.SelectionColor = HighlightColors.HC_STRING;
                        }

                    switch (state)
                    {
                        case 0:
                            if (!Char.IsWhiteSpace(nodeText, i))
                            {
                                startNodeName = i;
                                state = 1;
                            }
                            break;
                        case 1:
                            if (Char.IsWhiteSpace(nodeText, i))
                            {
                                rtb.Select(startNodeName + st, i - startNodeName + 1);
                                rtb.SelectionColor = HighlightColors.HC_NODE;
                                state = 2;
                            }
                            break;
                        case 2:
                            if (!Char.IsWhiteSpace(nodeText, i))
                            {
                                startAtt = i;
                                state = 3;
                            }
                            break;

                        case 3:
                            if (Char.IsWhiteSpace(nodeText, i) || nodeText[i] == '=')
                            {
                                rtb.Select(startAtt + st, i - startAtt + 1);
                                rtb.SelectionColor = HighlightColors.HC_ATTRIBUTE;
                                state = 4;
                            }
                            break;
                        case 4:
                            if (nodeText[i] == '"' && !inString)
                                state = 2;
                            break;
                    }
                }
                if (state == 1)
                {
                    rtb.Select(st + 1, nodeText.Length);
                    rtb.SelectionColor = HighlightColors.HC_NODE;
                }
            }
        }
    }

}