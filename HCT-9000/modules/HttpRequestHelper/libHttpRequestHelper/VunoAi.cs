using System;
using System.Collections.Generic;
using Newtonsoft.Json;
using System.IO;
using System.Net;
using System.Diagnostics;
using System.Windows.Forms;

namespace libHttpRequestHelper
{
    public class VunoAi
    {
        public static bool registerLicense(String serverUrl, String licenseKey)
        {
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(serverUrl);
            request.ContentType = "application/json";
            request.Method = "POST";
            request.KeepAlive = true;
            request.UserAgent = ".NET Framework Client";
            request.Credentials = System.Net.CredentialCache.DefaultCredentials;

            Stream rs = request.GetRequestStream();

            String formitem = "{\"license\": \"" + licenseKey + "\"}";
            byte[] formitembytes = System.Text.Encoding.UTF8.GetBytes(formitem);
            rs.Write(formitembytes, 0, formitembytes.Length);

            String strResponse = "";
            if (!getResponse(ref strResponse, request))
                return false;

            return true;
        }

        public static bool downloadLicenseInfo(String serverUrl, String resultFilePath)
        {
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(serverUrl);
            request.Method = "GET";
            request.KeepAlive = true;
            request.UserAgent = ".NET Framework Client";
            request.Credentials = System.Net.CredentialCache.DefaultCredentials;

            String strResponse = "";
            if (!getResponse(ref strResponse, request))
            {
                if (strResponse.Length <= 0)
                {
                    return false;
                }
            }

            File.WriteAllText(resultFilePath, strResponse);

            return true;
        }

        public static String getUniqueKey(String serverUrl)
        {
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(serverUrl);
            request.Method = "GET";
            request.KeepAlive = true;
            request.UserAgent = ".NET Framework Client";
            request.Credentials = System.Net.CredentialCache.DefaultCredentials;

            String strResponse = "";
            if (!getResponse(ref strResponse, request))
                return null;

            dynamic array = JsonConvert.DeserializeObject(strResponse);
            if (array == null)
                return null;

            return array["unique_key"];
        }

        public static bool requestAnalysis(String serverUrl, String fileName, String filePath, String resultFilePath)
        {
            string boundary = "--" + Guid.NewGuid().ToString("N") + "--";
            byte[] boundarybytes = System.Text.Encoding.ASCII.GetBytes("\r\n--" + boundary + "\r\n");

            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(serverUrl);
            request.ContentType = "multipart/form-data; boundary=" + boundary;
            request.Method = "POST";
            request.KeepAlive = true;
            request.UserAgent = ".NET Framework Client";
            request.Credentials = System.Net.CredentialCache.DefaultCredentials;

            Stream rs = request.GetRequestStream();

            string formdataTemplate = "Content-Disposition: form-data; name=\"{0}\"\r\n\r\n{1}";

            // filename
            {
                rs.Write(boundarybytes, 0, boundarybytes.Length);
                string formitem = string.Format(formdataTemplate, "file_name", fileName);
                byte[] formitembytes = System.Text.Encoding.UTF8.GetBytes(formitem);
                rs.Write(formitembytes, 0, formitembytes.Length);
            }

            // settings
            {
                var settings = new List<Tuple<String, int>>()
                {
                    new Tuple<String, int>("Drusen & Drusenoid Deposits", 2),
                    new Tuple<String, int>("Hemorrhage", 2),
                    new Tuple<String, int>("Hard Exudate", 2),
                    new Tuple<String, int>("Cotton Wool Patch", 2),
                    new Tuple<String, int>("Vascular Abnormality", 2),
                    new Tuple<String, int>("Glaucomatous Disc Change", 2),
                    new Tuple<String, int>("RNFL Defect", 2),
                    new Tuple<String, int>("Membrane", 2),
                    new Tuple<String, int>("Chorioretinal Atrophy/Scar", 2),
                    new Tuple<String, int>("Non-glaucomatous Disc Change", 2),
                    new Tuple<String, int>("Macular Hole", 2),
                    new Tuple<String, int>("Myelinated Nerve Fiber", 2),
                };

                String strSettings = "";
                strSettings += "[";
                foreach (var item in settings)
                {
                    strSettings += "{\"name\":\"";
                    strSettings += item.Item1;
                    strSettings += "\", \"sensitivity\": ";
                    strSettings += item.Item2;
                    strSettings += "},";
                }
                strSettings = strSettings.Remove(strSettings.Length - 1);
                strSettings += "]";

                rs.Write(boundarybytes, 0, boundarybytes.Length);
                string formitem = string.Format(formdataTemplate, "settings", strSettings);
                byte[] formitembytes = System.Text.Encoding.UTF8.GetBytes(formitem);
                rs.Write(formitembytes, 0, formitembytes.Length);
            }

            // force_gradable
            {
                rs.Write(boundarybytes, 0, boundarybytes.Length);
                string formitem = string.Format(formdataTemplate, "force_gradable", "False");
                byte[] formitembytes = System.Text.Encoding.UTF8.GetBytes(formitem);
                rs.Write(formitembytes, 0, formitembytes.Length);
            }

            // file_data
            {
                rs.Write(boundarybytes, 0, boundarybytes.Length);

                string headerTemplate = "Content-Disposition: form-data; name=\"{0}\"; filename=\"{1}\"\r\nContent-Type: {2}\r\n\r\n";
                string header = string.Format(headerTemplate, "file_data", "file_data", "image/jpg");
                byte[] headerbytes = System.Text.Encoding.UTF8.GetBytes(header);
                rs.Write(headerbytes, 0, headerbytes.Length);

                FileStream fileStream = new FileStream(filePath, FileMode.Open, FileAccess.Read);
                byte[] buffer = new byte[fileStream.Length];
                int bytesRead = fileStream.Read(buffer, 0, buffer.Length);

                rs.Write(buffer, 0, bytesRead);

                fileStream.Close();
            }

            //
            {
                byte[] trailer = System.Text.Encoding.ASCII.GetBytes("\r\n--" + boundary + "--\r\n");
                rs.Write(trailer, 0, trailer.Length);
                rs.Close();
            }

            // request
            String strResponse = "";
            if (!getResponse(ref strResponse, request))
                return false;

            File.WriteAllText(resultFilePath, strResponse);

            return true;
        }

        static bool getResponse(ref String out_strResp, HttpWebRequest request)
        {
            WebResponse resp = null;
            try
            {
                resp = request.GetResponse();
                StreamReader reader = new StreamReader(resp.GetResponseStream());
                out_strResp = reader.ReadToEnd();

                resp.Close();
                resp = null;

                return true;
            }
            catch (WebException we)
            {
                HttpWebResponse resp_err = we.Response as HttpWebResponse;
                if (resp_err != null && resp_err.StatusCode == HttpStatusCode.Unauthorized)
                {
                    out_strResp = @"{ 'err': 401 }";
                }

                if (resp != null)
                {
                    resp.Close();
                    resp = null;
                }

                return false;
            }
        }
    }
}
