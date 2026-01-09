// React Native packages that need to be transformed by Babel
const reactNativePackagesToTransform = [
  'react-native',
  '@react-native',
  '@react-navigation',
  'react-native-svg',
  'react-native-qrcode-svg',
  'expo',
  '@expo',
  '@noble',
  '@scure',
];

module.exports = {
  preset: 'jest-expo',
  moduleFileExtensions: ['ts', 'tsx', 'js', 'jsx', 'json', 'node'],
  setupFiles: ['<rootDir>/jest.setup.js'],
  transformIgnorePatterns: [`node_modules/(?!(${reactNativePackagesToTransform.join('|')})/)`],
  transform: {
    '^.+\\.(js|jsx|ts|tsx)$': 'babel-jest',
  },
  moduleNameMapper: {
    '^@components/(.*)$': '<rootDir>/src/components/$1',
    '^@components$': '<rootDir>/src/components',
    '^@screens/(.*)$': '<rootDir>/src/screens/$1',
    '^@screens$': '<rootDir>/src/screens',
    '^@services/(.*)$': '<rootDir>/src/services/$1',
    '^@services$': '<rootDir>/src/services',
    '^@store/(.*)$': '<rootDir>/src/store/$1',
    '^@store$': '<rootDir>/src/store',
    '^@utils/(.*)$': '<rootDir>/src/utils/$1',
    '^@utils$': '<rootDir>/src/utils',
    '^@constants/(.*)$': '<rootDir>/src/constants/$1',
    '^@constants$': '<rootDir>/src/constants',
    '^@types/(.*)$': '<rootDir>/src/types/$1',
    '^@types$': '<rootDir>/src/types',
    '^@navigation/(.*)$': '<rootDir>/src/navigation/$1',
    '^@navigation$': '<rootDir>/src/navigation',
    '\\.svg$': '<rootDir>/__mocks__/svgMock.js',
  },
  collectCoverageFrom: ['src/**/*.{ts,tsx}', '!src/**/*.d.ts', '!src/types/**'],
  coverageThreshold: {
    global: {
      statements: 70,
      branches: 60,
      functions: 70,
      lines: 70,
    },
  },
  testMatch: ['**/__tests__/**/*.test.(ts|tsx|js)'],
};
