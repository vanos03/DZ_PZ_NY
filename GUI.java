import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;


public class GUI extends JFrame {

    ImageIcon newIcon2 = new ImageIcon("img/2.png");
    Image image2 = newIcon2.getImage();
    Image newimg2 = image2.getScaledInstance(60, 60,  java.awt.Image.SCALE_SMOOTH);
    ImageIcon icon = new ImageIcon(newimg2);

    ImageIcon newIcon3 = new ImageIcon("img/3.png");
    Image image3 = newIcon3.getImage();
    Image newimg3 = image3.getScaledInstance(60, 60,  java.awt.Image.SCALE_SMOOTH);
    ImageIcon icon1 = new ImageIcon(newimg3);

    ImageIcon newIcon4 = new ImageIcon("img/4.png");
    Image image4 = newIcon4.getImage();
    Image newimg4 = image4.getScaledInstance(60, 60,  java.awt.Image.SCALE_SMOOTH);
    ImageIcon icon2 = new ImageIcon(newimg4);

    public GUI() {
        setTitle("ЭЦП ГОСТ 34.10-94");
        setSize(300, 340);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);

        String str1 = "<html><b> Опции: </font></html>";
        JLabel tf1 = new JLabel(str1, SwingConstants.CENTER);



        JButton signFileButton = new JButton("<html><b> Подписать файл </font></html>");
        signFileButton.setFocusPainted(false);
        signFileButton.setForeground(new Color(10, 50, 32));
        signFileButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String inputFilePath = selectFile("Выберите файл для подписи");
                System.out.println(inputFilePath);

                JDialog jd1 = new JDialog();
                jd1.setTitle("Выбор файла:");

                GOST341094 subfile = new GOST341094(inputFilePath);
                if (subfile.subscribe_file() == 0) {
                    String msg18 = "<html><font color = 'green'> Процесс успешно завершен: <br/> <font color = 'black'> <br/> Файл <br/> " +
                            inputFilePath + " <br/> <br/> успешно подписан ";
                    JOptionPane.showMessageDialog(jd1, msg18, null, 0, icon);
                }
                else{
                    String msg18 = "<html><font color = 'red'> Ошибка при попытке подписать файл: <br/> <font color = 'black'> <br/> Файл <br/> " + inputFilePath;
                    JOptionPane.showMessageDialog(jd1, msg18, null, 0, icon);
                }
            }
        });


        JButton verifySignatureButton = new JButton("<html><b> Проверка подписи </font></html>");
        verifySignatureButton.setFocusPainted(false);
        verifySignatureButton.setForeground(new Color(110, 22, 22));
        verifySignatureButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String inputFilePath = selectFile("Выберите файл для проверки (all)");
//                String pubkeyFilePath = selectFile("Выберите файл с открытым ключом (.pub)");
                String sigKeyFilePath = selectFile("Выберите файл с электронной подписью (.sig)");
//                String pqaFile = selectFile("Выберите файл с параметрами системы (.pqa)");


                GOST341094 subCheckFile = new GOST341094(inputFilePath, sigKeyFilePath);
                subCheckFile.check_signature_file();

                if(subCheckFile.check_signature_file() == 0){
                    JDialog jd2 = new JDialog();
                    jd2.setTitle("Электронная подпись действительна для этого файла");
                    String msg14 = "<html><font color = 'black'> Проверка завершена: <br/> <font color = 'green'> Подпись " +
                            "корректна! </font></html>";
                    JOptionPane.showMessageDialog(jd2, msg14, null, 0, icon1);
                }
                else{
                    JDialog jd2 = new JDialog();
                    jd2.setTitle("Электронная подпись недействительна для этого файла");
                    String msg14 = "<html><font color = 'black'> Проверка завершена: <br/> <font color = 'red'> Подпись " +
                            "некорректна! </font></html>";
                    JOptionPane.showMessageDialog(jd2, msg14, null, 0, icon2);
                }

            }
        });


        JPanel panel = new JPanel(new GridLayout(4, 1));
        setResizable(true);
        panel.setPreferredSize(new Dimension(4*100, 4*100));
        panel.setBackground(new Color(174, 176, 176));

        panel.add(tf1);
        panel.add(signFileButton);
        panel.add(verifySignatureButton);
        getContentPane().add(panel);

        pack();
        setVisible(true);
    }

    public String selectFile(String message) {

        File selectedFile = null;

        JFileChooser fileChooser = new JFileChooser(new File("./"));
        fileChooser.setDialogTitle(message);


        int result = fileChooser.showOpenDialog(this);

        if (result == JFileChooser.APPROVE_OPTION) {
            selectedFile = fileChooser.getSelectedFile();
            processSelectedFile(selectedFile);
        }

        return selectedFile.getAbsolutePath();
    }


    private void processSelectedFile(File file) {
        JOptionPane.showMessageDialog(this, "Файл: " + file.getAbsolutePath());
    }


    public static void main(String[] args) {

        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                new GUI();
            }
        });
    }

}